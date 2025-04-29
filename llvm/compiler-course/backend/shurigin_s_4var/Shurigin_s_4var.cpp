#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "shurigin-fmsub-combine"

using namespace llvm;

namespace {

class SUBPass : public MachineFunctionPass {
public:
  static char ID;
  SUBPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return "Simple Fused Multiply-Subtract Combine (Shurigin)";
  }

private:
  bool isSupportedMul(unsigned Opcode) const {
    switch (Opcode) {
    case X86::VMULSSrr:
    case X86::VMULSDrr:
    case X86::VMULPSrr:
    case X86::VMULPDrr:
      return true;
    default:
      return false;
    }
  }

  bool isSupportedSub(unsigned Opcode) const {
    switch (Opcode) {
    case X86::VSUBSSrr:
    case X86::VSUBSDrr:
    case X86::VSUBPSrr:
    case X86::VSUBPDrr:
      return true;
    default:
      return false;
    }
  }

  unsigned getFMAOpcode(unsigned SubOpcode) const {
    switch (SubOpcode) {
    case X86::VSUBSSrr:
      return X86::VFNMADD213SSr;
    case X86::VSUBSDrr:
      return X86::VFNMADD213SDr;
    case X86::VSUBPSrr:
      return X86::VFNMADD213PSr;
    case X86::VSUBPDrr:
      return X86::VFNMADD213PDr;
    default:
      return 0;
    }
  }
};

char SUBPass::ID = 0;

bool SUBPass::runOnMachineFunction(MachineFunction &MF) {

  const X86Subtarget &Subtarget = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = Subtarget.getInstrInfo();
  LLVM_DEBUG(dbgs() << "\nRunning SUBPass on function: " << MF.getName()
                    << '\n');

  if (!Subtarget.hasFMA()) {
    LLVM_DEBUG(dbgs() << "  Target does not support FMA. Skipping pass.\n");
    return false;
  }

  bool Changed = false;
  SmallVector<MachineInstr *, 4> InstrsToDelete;

  for (MachineBasicBlock &MBB : MF) {
    LLVM_DEBUG(dbgs() << " Processing MBB: " << MBB.getName() << '\n');
    InstrsToDelete.clear();

    for (auto MBBI = MBB.begin(), MBBE = MBB.end(); MBBI != MBBE; ++MBBI) {
      MachineInstr &SubMI = *MBBI;

      LLVM_DEBUG(dbgs() << "  Checking instruction: "; SubMI.dump(););

      if (!isSupportedSub(SubMI.getOpcode())) {
        LLVM_DEBUG(dbgs() << "    Not a supported AVX SUB opcode.\n");
        continue;
      }

      if (MBBI == MBB.begin()) {
        LLVM_DEBUG(
            dbgs() << "    SUB is the first instruction in the block.\n");
        continue;
      }
      MachineInstr *MulMIPtr = &*std::prev(MBBI);
      if (!MulMIPtr)
        continue;
      MachineInstr &MulMI = *MulMIPtr;

      LLVM_DEBUG(dbgs() << "    Preceding instruction: "; MulMI.dump(););

      if (!isSupportedMul(MulMI.getOpcode())) {
        LLVM_DEBUG(dbgs() << "    Preceding instruction is not a supported AVX "
                             "MUL opcode.\n");
        continue;
      }

      if (MulMI.getNumExplicitOperands() < 3 || !MulMI.getOperand(0).isReg() ||
          !MulMI.getOperand(1).isReg() || !MulMI.getOperand(2).isReg()) {
        LLVM_DEBUG(
            dbgs()
            << "    MUL operands mismatch expected 3-register pattern.\n");
        continue;
      }
      if (SubMI.getNumExplicitOperands() < 3 || !SubMI.getOperand(0).isReg() ||
          !SubMI.getOperand(1).isReg() || !SubMI.getOperand(2).isReg()) {
        LLVM_DEBUG(
            dbgs()
            << "    SUB operands mismatch expected 3-register pattern.\n");
        continue;
      }

      Register MulDstReg = MulMI.getOperand(0).getReg();
      Register AReg = MulMI.getOperand(1).getReg();
      Register BReg = MulMI.getOperand(2).getReg();

      Register DstReg = SubMI.getOperand(0).getReg();
      Register CReg = SubMI.getOperand(1).getReg();
      Register TmpReg = SubMI.getOperand(2).getReg();

      if (MulDstReg != TmpReg) {
        LLVM_DEBUG(dbgs() << "    SUB's second operand register doesn't match "
                             "MUL's destination register.\n");
        continue;
      }

      if (!SubMI.getOperand(2).isKill()) {
        LLVM_DEBUG(dbgs() << "    Safety check failed: TmpReg is not killed by "
                             "SUB. Skipping optimization.\n");
        continue;
      }

      unsigned FMAOpcode = getFMAOpcode(SubMI.getOpcode());
      if (FMAOpcode == 0) {
        LLVM_DEBUG(dbgs() << "    Could not find matching FMA opcode for SUB: "
                          << TII->getName(SubMI.getOpcode()) << "\n");
        continue;
      }

      LLVM_DEBUG(dbgs() << "  Pattern Found! Replacing with FMA.\n");
      LLVM_DEBUG(dbgs() << "    Using FMA Opcode: " << TII->getName(FMAOpcode)
                        << "\n");

      MachineInstr *NewMI =
          BuildMI(MBB, SubMI, SubMI.getDebugLoc(), TII->get(FMAOpcode), DstReg)
              .addReg(AReg, getRegState(MulMI.getOperand(1)))
              .addReg(BReg, getRegState(MulMI.getOperand(2)))
              .addReg(CReg, getRegState(SubMI.getOperand(1)))
              .getInstr();
      (void)NewMI;

      LLVM_DEBUG(dbgs() << "  New instruction created: "; NewMI->dump(););

      InstrsToDelete.push_back(&MulMI);
      InstrsToDelete.push_back(&SubMI);

      Changed = true;
      LLVM_DEBUG(dbgs() << "  Marked instructions for deletion.\n");
    }

    if (!InstrsToDelete.empty()) {
      LLVM_DEBUG(dbgs() << " Deleting marked instructions for MBB "
                        << MBB.getName() << "\n");
      for (MachineInstr *MI : reverse(InstrsToDelete)) {
        LLVM_DEBUG(dbgs() << "  Deleting: "; MI->dump(););
        MI->eraseFromParent();
      }
    }
  }

  LLVM_DEBUG(dbgs() << "SUBPass finished. Changes made: " << Changed << "\n");
  return Changed;
}

} // namespace

static RegisterPass<SUBPass>
    X(DEBUG_TYPE, "Simple Fused Multiply-Subtract Combine (Shurigin)", false,
      false);
