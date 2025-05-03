#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class MultiplySubtractPass : public MachineFunctionPass {
public:
  static char ID;
  MultiplySubtractPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const X86InstrInfo *TII;
  bool runOnMachineBasicBlock(MachineBasicBlock &MBB);
};

char MultiplySubtractPass::ID = 0;

bool MultiplySubtractPass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &STI = MF.getSubtarget<X86Subtarget>();
  TII = STI.getInstrInfo();

  bool Modified = false;
  for (MachineBasicBlock &MBB : MF) {
    Modified |= runOnMachineBasicBlock(MBB);
  }
  return Modified;
}

bool MultiplySubtractPass::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
  bool Modified = false;

  for (MachineInstr &MI : make_early_inc_range(MBB)) {
    // SUB: SUBSSrr, SUBPSrr, VSUBPSYrr, VSUBSDrr, VSUBPDYrr
    unsigned SubOpc = MI.getOpcode();
    if (SubOpc != X86::SUBSSrr && SubOpc != X86::SUBPSrr &&
        SubOpc != X86::VSUBPSYrr && SubOpc != X86::VSUBSDrr &&
        SubOpc != X86::VSUBPDYrr) {
      continue;
    }

    MachineOperand &Src1 = MI.getOperand(1); // c (from c - x)
    MachineOperand &Src2 = MI.getOperand(2); // x (from c - x)
    if (!Src1.isReg() || !Src2.isReg()) {
      continue;
    }

    MachineInstr *MulMI = nullptr;
    MachineBasicBlock::iterator I = MBB.begin();
    MachineBasicBlock::iterator E = MI.getIterator();
    for (; I != E; ++I) {
      MachineInstr &PrevMI = *I;
      if (PrevMI.getOpcode() == X86::MULSSrr ||
          PrevMI.getOpcode() == X86::MULPSrr ||
          PrevMI.getOpcode() == X86::VMULPSYrr ||
          PrevMI.getOpcode() == X86::MULSDrr ||
          PrevMI.getOpcode() == X86::VMULPDYrr) {
        if (PrevMI.getOperand(0).getReg() == Src2.getReg()) {
          MulMI = &PrevMI;
          break;
        }
      }

      else if (PrevMI.getOpcode() == X86::VMULPSYrm ||
               PrevMI.getOpcode() == X86::VMULPDYrm) {
        continue;
      }
    }

    if (!MulMI) {
      continue;
    }

    MachineOperand &MulOp1 = MulMI->getOperand(1); // a
    MachineOperand &MulOp2 = MulMI->getOperand(2); // b
    if (!MulOp1.isReg() || !MulOp2.isReg()) {
      continue;
    }

    unsigned FMSUBOpcode;
    if (SubOpc == X86::SUBSSrr) {
      FMSUBOpcode = X86::VFMSUB213SSr;
    } else if (SubOpc == X86::VSUBSDrr) {
      FMSUBOpcode = X86::VFMSUB213SDr;
    } else if (SubOpc == X86::VSUBPDYrr) {
      FMSUBOpcode = X86::VFMSUB213PDYr; // VFMSUB213PDrr
    } else {
      FMSUBOpcode = X86::VFMSUB213PSr; // SUBPSrr, VSUBPSYrr
    }

    BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(FMSUBOpcode),
            MI.getOperand(0).getReg())
        .addReg(Src1.getReg())   // c
        .addReg(MulOp1.getReg()) // a
        .addReg(MulOp2.getReg()) // b
        .addImm(0)               // rounding mode
        .addReg(X86::MXCSR, RegState::Implicit)
        .addReg(X86::MXCSR, RegState::Implicit);

    MulMI->eraseFromParent();
    MI.eraseFromParent();
    Modified = true;
  }

  return Modified;
}
} // namespace

static RegisterPass<MultiplySubtractPass>
    X("mult_sub_pass", "fused multiply-subtract", false, false);
