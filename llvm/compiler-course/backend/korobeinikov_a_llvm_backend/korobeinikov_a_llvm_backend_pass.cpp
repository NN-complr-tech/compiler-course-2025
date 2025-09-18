#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"

using namespace llvm;

namespace {
class FMACombinePass : public MachineFunctionPass {
public:
  static char ID;
  FMACombinePass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const X86InstrInfo *TII = nullptr;

  bool processBlock(MachineBasicBlock &MBB);
  std::optional<unsigned> getFMAOpcode(unsigned MulOp, unsigned SubOp,
                                       bool IsSubLHSfirst) const;
  bool checkRegisterUsage(const MachineInstr &MulMI, const MachineInstr &SubMI,
                          bool &IsSubLHSfirst) const;
};

char FMACombinePass::ID = 0;

bool FMACombinePass::runOnMachineFunction(MachineFunction &MF) {
  auto &ST = MF.getSubtarget<X86Subtarget>();
  if (!ST.hasFMA())
    return false;
  TII = ST.getInstrInfo();

  bool Changed = false;
  for (auto &MBB : MF)
    Changed |= processBlock(MBB);
  return Changed;
}

bool FMACombinePass::processBlock(MachineBasicBlock &MBB) {
  SmallVector<MachineInstr *, 4> ToErase;
  auto &MRI = MBB.getParent()->getRegInfo();
  bool Modified = false;

  for (auto I = MBB.begin(), E = MBB.end(); I != E; ++I) {
    MachineInstr &MulMI = *I;
    unsigned MulOp = MulMI.getOpcode();
    // 1) Ищем fmul
    if (MulOp != X86::MULSSrr && MulOp != X86::MULSDrr &&
        MulOp != X86::VMULSSrr && MulOp != X86::VMULSDrr &&
        MulOp != X86::VMULPSrr && MulOp != X86::VMULPDrr)
      continue;

    // ровно одно использование результата mul
    Register MulDst = MulMI.getOperand(0).getReg();
    if (!MRI.hasOneUse(MulDst))
      continue;

    // 2) Ищем sub или add вперед (теперь ловим и tmp во 2-м операнде для add)
    bool IsSubLHSfirst = false;
    auto SubIt = std::next(I);
    for (; SubIt != E; ++SubIt) {
      MachineInstr &SubMI = *SubIt;
      unsigned Op = SubMI.getOpcode();
      bool isSub = Op == X86::SUBSSrr || Op == X86::SUBSDrr ||
                   Op == X86::VSUBSSrr || Op == X86::VSUBSDrr;
      bool isAdd = Op == X86::VADDSSrr || Op == X86::VADDSDrr;

      if (isSub) {
        // tmp-c (tmp в operand1) или c-tmp (tmp в operand2)
        if (SubMI.getOperand(1).getReg() == MulDst ||
            SubMI.getOperand(2).getReg() == MulDst)
          break;
      } else if (isAdd) {
        // два варианта:
        //   -(tmp)+c  → tmp в operand1
        //    c+tmp    → tmp в operand2
        if (SubMI.getOperand(1).getReg() == MulDst ||
            SubMI.getOperand(2).getReg() == MulDst)
          break;
      }
    }

    if (SubIt == E)
      continue;
    MachineInstr &SubMI = *SubIt;

    // 3) Определяем, какой у нас случай, и корректность использования
    if (!checkRegisterUsage(MulMI, SubMI, IsSubLHSfirst))
      continue;

    // 4) Получаем нужный FMA opcode
    auto FMAOp = getFMAOpcode(MulOp, SubMI.getOpcode(), IsSubLHSfirst);
    if (!FMAOp)
      continue;

    // 5) Выбираем индекс региста c
    //   если tmp в lhs → c = operand2, иначе c = operand1
    unsigned CIdx = IsSubLHSfirst ? 2 : 1;
    Register CReg = SubMI.getOperand(CIdx).getReg();

    // 6) Строим fused-инструкцию
    BuildMI(MBB, SubMI, SubMI.getDebugLoc(), TII->get(*FMAOp),
            SubMI.getOperand(0).getReg())
        .addReg(MulMI.getOperand(1).getReg(), getRegState(MulMI.getOperand(1)))
        .addReg(MulMI.getOperand(2).getReg(), getRegState(MulMI.getOperand(2)))
        .addReg(CReg, getRegState(SubMI.getOperand(CIdx)));

    ToErase.push_back(&MulMI);
    ToErase.push_back(&SubMI);
    Modified = true;
  }

  for (auto *MI : reverse(ToErase))
    MI->eraseFromParent();
  return Modified;
}

std::optional<unsigned> FMACombinePass::getFMAOpcode(unsigned MulOp,
                                                     unsigned SubOp,
                                                     bool IsSubLHSfirst) const {
  // Если это ADD-случай (-(a*b)+c), всегда VFNMADD213
  if (SubOp == X86::VADDSSrr)
    return IsSubLHSfirst ? X86::VFNMADD213SSr : X86::VFNMADD132SSr;
  if (SubOp == X86::VADDSDrr)
    return IsSubLHSfirst ? X86::VFNMADD213SDr : X86::VFNMADD132SDr;

  // Иначе SUB-случай:
  //   если tmp в lhs → (a*b)-c = VFMSUB213
  //   иначе           c-(a*b) = VFNMADD213
  bool tmpOnLHS = IsSubLHSfirst;
  switch (MulOp) {
  case X86::MULSSrr:
    return tmpOnLHS ? X86::VFMSUB213SSr : X86::VFNMADD213SSr;
  case X86::VMULSSrr:
    return tmpOnLHS ? X86::VFMSUB213SSr : X86::VFNMADD213SSr;
  case X86::MULSDrr:
    return tmpOnLHS ? X86::VFMSUB213SDr : X86::VFNMADD213SDr;
  case X86::VMULSDrr:
    return tmpOnLHS ? X86::VFMSUB213SDr : X86::VFNMADD213SDr;
  case X86::VMULPSrr:
    return tmpOnLHS ? X86::VFMSUB213PSr : X86::VFNMADD213PSr;
  case X86::VMULPDrr:
    return tmpOnLHS ? X86::VFMSUB213PDr : X86::VFNMADD213PDr;
  default:
    return std::nullopt;
  }
}

bool FMACombinePass::checkRegisterUsage(const MachineInstr &MulMI,
                                        const MachineInstr &SubMI,
                                        bool &IsSubLHSfirst) const {
  Register MulDst = MulMI.getOperand(0).getReg();
  unsigned Op = SubMI.getOpcode();
  Register LHS = SubMI.getOperand(1).getReg();
  Register RHS = SubMI.getOperand(2).getReg();

  if (Op == X86::VADDSSrr || Op == X86::VADDSDrr) {
    // ADD-case:
    //   -(tmp)+c → tmp in LHS  → IsSubLHSfirst = true
    //    c + (tmp) → tmp in RHS → IsSubLHSfirst = false
    if (LHS == MulDst) {
      IsSubLHSfirst = true;
      return true;
    }
    if (RHS == MulDst) {
      IsSubLHSfirst = false;
      return true;
    }
    return false;
  }
  // SUB-case:
  if (LHS == MulDst) {
    // tmp - c
    IsSubLHSfirst = true;
    return true;
  }
  if (RHS == MulDst) {
    // c - tmp
    IsSubLHSfirst = false;
    return true;
  }
  return false;
}

} // namespace

static RegisterPass<FMACombinePass>
    X("korobeinikov-fuse-mul-sub", "Fuse mul+sub/add into VFMSUB213/VFNMADD213",
      false, false);
