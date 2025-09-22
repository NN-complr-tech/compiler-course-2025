#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "fused-mul-sub-khokhlov"

using namespace llvm;

namespace {
class FusedMulSubPass : public MachineFunctionPass {
public:
  static char ID;
  FusedMulSubPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &Func) override;

private:
  const X86InstrInfo *InstrDesc = nullptr;

  bool transformBlock(MachineBasicBlock &BasicBlock);
  std::optional<unsigned> pickFMAOpcode(unsigned MulCode, unsigned SubCode,
                                        bool MulResultOnLeft) const;
  bool verifyRegisterUsage(const MachineInstr &MulInstruction,
                           const MachineInstr &SubInstruction,
                           bool &MulResultOnLeft) const;
};

char FusedMulSubPass::ID = 0;

bool FusedMulSubPass::runOnMachineFunction(MachineFunction &Func) {
  const auto &Target = Func.getSubtarget<X86Subtarget>();
  if (!Target.hasFMA())
    return false;
  InstrDesc = Target.getInstrInfo();

  bool Altered = false;
  for (MachineBasicBlock &BasicBlock : Func)
    Altered |= transformBlock(BasicBlock);
  return Altered;
}

bool FusedMulSubPass::transformBlock(MachineBasicBlock &BasicBlock) {
  SmallVector<MachineInstr *, 4> ToDelete;
  auto &RegInfo = BasicBlock.getParent()->getRegInfo();
  bool Changed = false;

  for (auto Itr = BasicBlock.begin(), End = BasicBlock.end(); Itr != End;
       ++Itr) {
    MachineInstr &MulInstr = *Itr;
    unsigned MulOpcode = MulInstr.getOpcode();

    // Проверяем, является ли инструкция умножением
    if (MulOpcode != X86::MULSSrr && MulOpcode != X86::MULSDrr &&
        MulOpcode != X86::VMULSSrr && MulOpcode != X86::VMULSDrr &&
        MulOpcode != X86::VMULPSrr && MulOpcode != X86::VMULPDrr) {
      LLVM_DEBUG(dbgs() << "Skipping non-multiply instruction: " << MulInstr << "\n");
      continue;
    }

    // Проверяем, что результат умножения используется один раз
    Register MulOutput = MulInstr.getOperand(0).getReg();
    if (!RegInfo.hasOneUse(MulOutput)) {
      LLVM_DEBUG(dbgs() << "Multiply result has multiple uses: " << MulOutput << "\n");
      continue;
    }

    // Ищем инструкцию вычитания или сложения
    bool MulResultOnLeft = false;
    auto NextInstr = std::next(Itr);
    for (; NextInstr != End; ++NextInstr) {
      MachineInstr &SubInstr = *NextInstr;
      unsigned SubOpcode = SubInstr.getOpcode();
      bool IsSub = SubOpcode == X86::SUBSSrr || SubOpcode == X86::SUBSDrr ||
                   SubOpcode == X86::VSUBSSrr || SubOpcode == X86::VSUBSDrr ||
                   SubOpcode == X86::VSUBPSrr;
      bool IsAdd = SubOpcode == X86::VADDSSrr || SubOpcode == X86::VADDSDrr;

      if (IsSub || IsAdd) {
        if (SubInstr.getOperand(1).getReg() == MulOutput ||
            SubInstr.getOperand(2).getReg() == MulOutput) {
          LLVM_DEBUG(dbgs() << "Found matching subtract/add instruction: " << SubInstr << "\n");
          break;
        }
      }
    }

    if (NextInstr == End) {
      LLVM_DEBUG(dbgs() << "No suitable subtract/add instruction found for: " << MulInstr << "\n");
      continue;
    }
    MachineInstr &SubInstr = *NextInstr;

    // Проверяем корректность использования регистров
    if (!verifyRegisterUsage(MulInstr, SubInstr, MulResultOnLeft)) {
      LLVM_DEBUG(dbgs() << "Invalid register usage for: " << MulInstr << " and " << SubInstr << "\n");
      continue;
    }

    // Выбираем подходящий FMA opcode
    auto FMAOpcode = pickFMAOpcode(MulOpcode, SubInstr.getOpcode(), MulResultOnLeft);
    if (!FMAOpcode) {
      LLVM_DEBUG(dbgs() << "No suitable FMA opcode found for: " << MulInstr << " and " << SubInstr << "\n");
      continue;
    }

    // Определяем индекс регистра для операнда C
    unsigned CIndex = MulResultOnLeft ? 2 : 1;
    Register COperand = SubInstr.getOperand(CIndex).getReg();

    // Создаем новую FMA-инструкцию
    LLVM_DEBUG(dbgs() << "Creating FMA instruction with opcode: " << *FMAOpcode << "\n");
    BuildMI(BasicBlock, SubInstr, SubInstr.getDebugLoc(),
            InstrDesc->get(*FMAOpcode), SubInstr.getOperand(0).getReg())
        .addReg(MulInstr.getOperand(1).getReg(),
                getRegState(MulInstr.getOperand(1)))
        .addReg(MulInstr.getOperand(2).getReg(),
                getRegState(MulInstr.getOperand(2)))
        .addReg(COperand, getRegState(SubInstr.getOperand(CIndex)));

    ToDelete.push_back(&MulInstr);
    ToDelete.push_back(&SubInstr);
    Changed = true;
  }

  // Удаляем старые инструкции
  for (auto *Instr : reverse(ToDelete))
    Instr->eraseFromParent();
  return Changed;
}

std::optional<unsigned>
FusedMulSubPass::pickFMAOpcode(unsigned MulCode, unsigned SubCode,
                               bool MulResultOnLeft) const {
  // Случай сложения: -(a*b) + c
  if (SubCode == X86::VADDSSrr)
    return MulResultOnLeft ? X86::VFNMADD213SSr : X86::VFNMADD132SSr;
  if (SubCode == X86::VADDSDrr)
    return MulResultOnLeft ? X86::VFNMADD213SDr : X86::VFNMADD132SDr;

  // Случай вычитания
  switch (MulCode) {
  case X86::MULSSrr:
  case X86::VMULSSrr:
    return MulResultOnLeft ? X86::VFMSUB213SSr : X86::VFNMADD213SSr;
  case X86::MULSDrr:
  case X86::VMULSDrr:
    return MulResultOnLeft ? X86::VFMSUB213SDr : X86::VFNMADD213SDr;
  case X86::VMULPSrr:
    return MulResultOnLeft ? X86::VFMSUB213PSr : X86::VFNMADD213PSr;
  case X86::VMULPDrr:
    return MulResultOnLeft ? X86::VFMSUB213PDr : X86::VFNMADD213PDr;
  default:
    return std::nullopt;
  }
}

bool FusedMulSubPass::verifyRegisterUsage(const MachineInstr &MulInstruction,
                                          const MachineInstr &SubInstruction,
                                          bool &MulResultOnLeft) const {
  Register MulOutput = MulInstruction.getOperand(0).getReg();
  unsigned SubOpcode = SubInstruction.getOpcode();
  Register LeftOp = SubInstruction.getOperand(1).getReg();
  Register RightOp = SubInstruction.getOperand(2).getReg();

  if (SubOpcode == X86::VADDSSrr || SubOpcode == X86::VADDSDrr) {
    // Случай сложения: -(tmp) + c или c + tmp
    if (LeftOp == MulOutput) {
      MulResultOnLeft = true;
      return true;
    }
    if (RightOp == MulOutput) {
      MulResultOnLeft = false;
      return true;
    }
    return false;
  }

  // Случай вычитания: tmp - c или c - tmp
  if (SubOpcode == X86::SUBSSrr || SubOpcode == X86::SUBSDrr ||
      SubOpcode == X86::VSUBSSrr || SubOpcode == X86::VSUBSDrr ||
      SubOpcode == X86::VSUBPSrr) {
    if (LeftOp == MulOutput) {
      MulResultOnLeft = true;
      return true;
    }
    if (RightOp == MulOutput) {
      MulResultOnLeft = false;
      return true;
    }
  }
  return false;
}

} // namespace

static RegisterPass<FusedMulSubPass>
    Z("fused-mul-sub-khokhlov",
      "Combine multiply and subtract/add into VFMSUB213/VFNMADD213", false,
      false);