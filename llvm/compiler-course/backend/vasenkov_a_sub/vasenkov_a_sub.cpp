#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "SubPass"

using namespace llvm;

namespace {
class MulSubPass : public MachineFunctionPass {
public:
  static char ID;
  MulSubPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    bool Modified = false;

    for (MachineBasicBlock &MBB : MF) {
      auto I = MBB.begin();
      auto E = MBB.end();

      while (I != E) {
        MachineInstr *CurrentMI = &*I;
        ++I;

        if (isCandidateForFusion(CurrentMI)) {
          if (performFusion(MBB, CurrentMI, TII)) {
            Modified = true;
            I = std::next(MBB.begin());
          }
        }
      }
    }

    return Modified;
  }

private:
  bool isCandidateForFusion(MachineInstr *MI) const {
    unsigned Opc = MI->getOpcode();
    return Opc == X86::VSUBSSrr || Opc == X86::VSUBSDrr;
  }

  bool performFusion(MachineBasicBlock &MBB, MachineInstr *SubInst,
                     const X86InstrInfo *TII) {
    MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

    if (SubInst->getNumOperands() < 3)
      return false;

    Register SecondOperand = SubInst->getOperand(2).getReg();
    MachineInstr *MulInst = MRI.getUniqueVRegDef(SecondOperand);

    if (!MulInst || !isValidMultiply(MulInst))
      return false;

    return replaceWithFusedInstruction(MBB, SubInst, MulInst, TII);
  }

  bool isValidMultiply(MachineInstr *MI) const {
    unsigned Opc = MI->getOpcode();
    return Opc == X86::VMULSSrr || Opc == X86::VMULSDrr;
  }

  bool replaceWithFusedInstruction(MachineBasicBlock &MBB,
                                   MachineInstr *SubInst, MachineInstr *MulInst,
                                   const X86InstrInfo *TII) {
    Register ResultReg = SubInst->getOperand(0).getReg();
    Register FirstSubOperand = SubInst->getOperand(1).getReg();
    Register FirstMulOperand = MulInst->getOperand(1).getReg();
    Register SecondMulOperand = MulInst->getOperand(2).getReg();
    DebugLoc Location = SubInst->getDebugLoc();

    bool IsSinglePrecision = SubInst->getOpcode() == X86::VSUBSSrr;
    unsigned FusedOpcode =
        IsSinglePrecision ? X86::VFNMADD213SSr : X86::VFNMADD213SDr;

    auto NewInstr =
        BuildMI(MBB, SubInst, Location, TII->get(FusedOpcode), ResultReg)
            .addReg(FirstSubOperand)
            .addReg(FirstMulOperand)
            .addReg(SecondMulOperand)
            .addReg(X86::MXCSR, RegState::Implicit);

    SubInst->eraseFromParent();
    MulInst->eraseFromParent();

    return true;
  }
};

char MulSubPass::ID = 0;

static RegisterPass<MulSubPass>
    Registration("SubPass", "Fuse multiply-subtract patterns", false, false);
} // namespace
