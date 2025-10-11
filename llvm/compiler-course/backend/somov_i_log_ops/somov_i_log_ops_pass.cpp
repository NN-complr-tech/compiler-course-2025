#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "x86-and-or-fusion"

using namespace llvm;

namespace {

class X86AndOrFusionPass : public MachineFunctionPass {
public:
  static char ID;
  X86AndOrFusionPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "X86 AND-OR Fusion Pass"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

char X86AndOrFusionPass::ID = 0;

bool X86AndOrFusionPass::runOnMachineFunction(MachineFunction &MachineFunc) {
  const X86Subtarget &Subtarget = MachineFunc.getSubtarget<X86Subtarget>();
  const X86InstrInfo *InstrInfo = Subtarget.getInstrInfo();
  MachineRegisterInfo &RegInfo = MachineFunc.getRegInfo();

  bool MadeChange = false;

  for (auto &Block : MachineFunc) {
    for (auto InstrIter = Block.begin(); InstrIter != Block.end();) {
      MachineInstr &AndCandidate = *InstrIter;
      unsigned FirstOpcode = AndCandidate.getOpcode();

      const bool isAndOp =
          (FirstOpcode == X86::ANDPSrr || FirstOpcode == X86::VPANDrr);
      if (!isAndOp) {
        ++InstrIter;
        continue;
      }

      if (!AndCandidate.getOperand(0).isReg() ||
          !AndCandidate.getOperand(1).isReg() ||
          !AndCandidate.getOperand(2).isReg()) {
        ++InstrIter;
        continue;
      }

      const Register IntermediateReg = AndCandidate.getOperand(0).getReg();
      const Register AndSource1 = AndCandidate.getOperand(1).getReg();
      const Register AndSource2 = AndCandidate.getOperand(2).getReg();

      auto NextInstrIter = std::next(InstrIter);
      if (NextInstrIter == Block.end()) {
        ++InstrIter;
        continue;
      }

      MachineInstr &OrCandidate = *NextInstrIter;
      unsigned SecondOpcode = OrCandidate.getOpcode();

      const bool isOrOp =
          (SecondOpcode == X86::ORPSrr || SecondOpcode == X86::VPORrr);
      if (!isOrOp) {
        ++InstrIter;
        continue;
      }

      const bool isPatternMatch =
          OrCandidate.getOperand(1).isReg() &&
          OrCandidate.getOperand(1).getReg() == IntermediateReg &&
          OrCandidate.getOperand(0).isReg() &&
          OrCandidate.getOperand(2).isReg();

      if (!isPatternMatch) {
        ++InstrIter;
        continue;
      }

      const Register FinalDestReg = OrCandidate.getOperand(0).getReg();
      const Register OrSource2 = OrCandidate.getOperand(2).getReg();

      LLVM_DEBUG(dbgs() << "Found a candidate AND/OR pair to fuse:\n"
                        << "  " << AndCandidate << "  " << OrCandidate);

      const TargetRegisterClass *Src1RegClass = RegInfo.getRegClass(AndSource1);
      const TargetRegisterClass *Src2RegClass = RegInfo.getRegClass(AndSource2);

      if (!Src1RegClass && !Src2RegClass) {
        LLVM_DEBUG(dbgs() << "Skipping: Could not determine register class.\n");
        continue;
      }
      if (Src1RegClass && Src2RegClass && Src1RegClass != Src2RegClass) {
        LLVM_DEBUG(dbgs() << "Skipping: Incompatible register classes.\n");
        continue;
      }

      const TargetRegisterClass *OperandRegClass =
          Src1RegClass ? Src1RegClass : Src2RegClass;

      const DebugLoc DbgLoc = AndCandidate.getDebugLoc();

      const Register NewIntermediateReg =
          RegInfo.createVirtualRegister(OperandRegClass);

      BuildMI(Block, AndCandidate, DbgLoc, InstrInfo->get(X86::VPANDrr),
              NewIntermediateReg)
          .addReg(AndSource1)
          .addReg(AndSource2);

      BuildMI(Block, OrCandidate, DbgLoc, InstrInfo->get(X86::VPORrr),
              FinalDestReg)
          .addReg(NewIntermediateReg)
          .addReg(OrSource2);

      LLVM_DEBUG(dbgs() << "Successfully fused instructions.\n");

      InstrIter = Block.erase(InstrIter);
      Block.erase(NextInstrIter);

      MadeChange = true;
    }
  }

  return MadeChange;
}

} // namespace

static RegisterPass<X86AndOrFusionPass>
    X("x86-and-or-fusion",
      "Fuses AND+OR instruction sequences into AVX equivalents", false, false);