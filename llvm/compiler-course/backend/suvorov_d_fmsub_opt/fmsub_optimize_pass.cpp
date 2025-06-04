#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "fmsub_pass"

using namespace llvm;

namespace {

class FMSUBPass : public MachineFunctionPass {
public:
  static char ID;
  FMSUBPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool isSupportedMul(unsigned opcode) const {
    switch (opcode) {
    case X86::VMULSSrr:
    case X86::VMULSDrr:
    case X86::VMULPSrr:
    case X86::VMULPDrr:
      return true;
    default:
      return false;
    }
  }

  bool isSupportedSub(unsigned opcode) const {
    switch (opcode) {
    case X86::VSUBSSrr:
    case X86::VSUBSDrr:
    case X86::VSUBPSrr:
    case X86::VSUBPDrr:
      return true;
    default:
      return false;
    }
  }

  unsigned getFMAOpcode(unsigned opcode) const {
    switch (opcode) {
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

char FMSUBPass::ID = 0;

bool FMSUBPass::runOnMachineFunction(MachineFunction &MF) {
  const X86Subtarget &Subtarget = MF.getSubtarget<X86Subtarget>();
  const X86InstrInfo *TII = Subtarget.getInstrInfo();
  LLVM_DEBUG(dbgs() << "Running FMSUBPass on function: " << MF.getName()
                    << '\n');

  if (!Subtarget.hasFMA()) {
    LLVM_DEBUG(dbgs() << "  Target does not support FMA. Skipping.\n");
    return false;
  }

  bool Changed = false;
  SmallVector<MachineInstr *, 16> InstrsToErase;

  for (MachineBasicBlock &MBB : MF) {
    InstrsToErase.clear();

    for (auto MBBInstrIt = MBB.begin(), MBBEnd = MBB.end();
         MBBInstrIt != MBBEnd; ++MBBInstrIt) {
      MachineInstr &SubMInstr = *MBBInstrIt;

      if (!isSupportedSub(SubMInstr.getOpcode()))
        continue;

      if (MBBInstrIt == MBB.begin())
        continue;

      MachineInstr &MulMInstr = *std::prev(MBBInstrIt);

      if (!isSupportedMul(MulMInstr.getOpcode()))
        continue;

      Register MulRes = MulMInstr.getOperand(0).getReg();
      Register A = MulMInstr.getOperand(1).getReg();
      Register B = MulMInstr.getOperand(2).getReg();

      Register SubRes = SubMInstr.getOperand(0).getReg();
      Register C = SubMInstr.getOperand(1).getReg();
      Register Tmp = SubMInstr.getOperand(2).getReg();

      if (MulRes != Tmp)
        continue;

      if (!SubMInstr.getOperand(2).isKill())
        continue;

      unsigned FMAOpcode = getFMAOpcode(SubMInstr.getOpcode());
      if (FMAOpcode == 0)
        continue;

      LLVM_DEBUG(dbgs() << "Found FMSUB pattern: "; SubMInstr.dump();
                 MulMInstr.dump(););

      MachineInstr *FMA = BuildMI(MBB, SubMInstr, SubMInstr.getDebugLoc(),
                                  TII->get(FMAOpcode), SubRes)
                              .addReg(A, getRegState(MulMInstr.getOperand(1)))
                              .addReg(B, getRegState(MulMInstr.getOperand(2)))
                              .addReg(C, getRegState(SubMInstr.getOperand(1)))
                              .getInstr();

      LLVM_DEBUG(dbgs() << "  Replaced with: "; FMA->dump(););

      InstrsToErase.push_back(&MulMInstr);
      InstrsToErase.push_back(&SubMInstr);
      Changed = true;
    }

    for (MachineInstr *MInst : reverse(InstrsToErase))
      MInst->eraseFromParent();
  }

  return Changed;
}

} // namespace

static RegisterPass<FMSUBPass> X(DEBUG_TYPE, "Suvorov FMSub Optimizer Pass",
                                 false, false);
