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
  std::optional<unsigned> getFMAOpcode(unsigned MulOp, unsigned SubOp) const;
  bool checkRegisterUsage(const MachineInstr &MulMI,
                          const MachineInstr &SubMI) const;
};

char FMACombinePass::ID = 0;

bool FMACombinePass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
  if (!ST.hasFMA())
    return false;

  TII = ST.getInstrInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF)
    Changed |= processBlock(MBB);

  return Changed;
}

bool FMACombinePass::processBlock(MachineBasicBlock &MBB) {
  bool Modified = false;
  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
  SmallVector<MachineInstr *, 4> ToErase;

  for (auto I = MBB.begin(); I != MBB.end(); ++I) {
    MachineInstr &MI = *I;
    unsigned MulOp = MI.getOpcode();

    if (MulOp != X86::MULSSrr && MulOp != X86::MULSDrr &&
        MulOp != X86::VMULSSrr && MulOp != X86::VMULSDrr &&
        MulOp != X86::VMULPSrr && MulOp != X86::VMULPDrr)
      continue;

    Register MulDst = MI.getOperand(0).getReg();
    if (!MRI.hasOneUse(MulDst))
      continue;

    auto SubIter = std::next(I);
    for (; SubIter != MBB.end(); ++SubIter) {
      MachineInstr &SubMI = *SubIter;
      unsigned Op = SubMI.getOpcode();
      bool isSub = (Op == X86::SUBSSrr || Op == X86::SUBSDrr ||
                    Op == X86::VSUBSSrr || Op == X86::VSUBSDrr);
      bool isAdd = (Op == X86::VADDSSrr || Op == X86::VADDSDrr);
      if ((isSub && SubMI.getOperand(2).getReg() == MulDst) ||
          (isAdd && SubMI.getOperand(1).getReg() == MulDst))
        break;
    }
    if (SubIter == MBB.end())
      continue;

    MachineInstr &SubMI = *SubIter;
    if (!checkRegisterUsage(MI, SubMI))
      continue;

    auto FMAOp = getFMAOpcode(MulOp, SubMI.getOpcode());
    if (!FMAOp)
      continue;

    bool isAdd = (SubMI.getOpcode() == X86::VADDSSrr ||
                  SubMI.getOpcode() == X86::VADDSDrr);
    unsigned CIdx = isAdd ? 2 : 1;
    Register CReg = SubMI.getOperand(CIdx).getReg();

    // Строим новую FMA инструкцию
    BuildMI(MBB, SubMI, SubMI.getDebugLoc(), TII->get(*FMAOp),
            SubMI.getOperand(0).getReg())
        .addReg(MI.getOperand(1).getReg(), getRegState(MI.getOperand(1)))
        .addReg(MI.getOperand(2).getReg(), getRegState(MI.getOperand(2)))
        .addReg(CReg, getRegState(SubMI.getOperand(CIdx)));

    // Отложенное удаление
    ToErase.push_back(&MI);
    ToErase.push_back(&SubMI);
    Modified = true;
  }

  if (!ToErase.empty()) {
    for (MachineInstr *MI : reverse(ToErase))
      MI->eraseFromParent();
  }

  return Modified;
}

std::optional<unsigned> FMACombinePass::getFMAOpcode(unsigned MulOp,
                                                     unsigned SubOp) const {
  const static DenseMap<std::pair<unsigned, unsigned>, unsigned> Map = {
      {{X86::MULSSrr, X86::SUBSSrr}, X86::VFMSUB213SSr},
      {{X86::VMULSSrr, X86::VSUBSSrr}, X86::VFMSUB213SSr},
      {{X86::MULSDrr, X86::SUBSDrr}, X86::VFMSUB213SDr},
      {{X86::VMULSDrr, X86::VSUBSDrr}, X86::VFMSUB213SDr},
      {{X86::VMULSSrr, X86::VADDSSrr}, X86::VFMSUB213SSr},
      {{X86::VMULSDrr, X86::VADDSDrr}, X86::VFMSUB213SDr},
      {{X86::VMULPSrr, X86::VSUBPSrr}, X86::VFMSUB213PSr},
      {{X86::VMULPDrr, X86::VSUBPDrr}, X86::VFMSUB213PDr},
  };
  auto It = Map.find({MulOp, SubOp});
  return It == Map.end() ? std::nullopt : std::optional<unsigned>(It->second);
}

bool FMACombinePass::checkRegisterUsage(const MachineInstr &MulMI,
                                        const MachineInstr &SubMI) const {
  Register MulDst = MulMI.getOperand(0).getReg();
  bool isAdd = (SubMI.getOpcode() == X86::VADDSSrr ||
                SubMI.getOpcode() == X86::VADDSDrr);
  Register SubSrc = SubMI.getOperand(isAdd ? 1 : 2).getReg();
  return MulDst == SubSrc;
}

} // namespace

static RegisterPass<FMACombinePass> X("korobeinikov-fuse-mul-sub",
                                      "FMA Combine Pass", false, false);
