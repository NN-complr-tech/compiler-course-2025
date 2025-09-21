#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class RemoveRedundantCopy : public MachineFunctionPass {
public:
  static char ID;
  RemoveRedundantCopy() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return "CompilerCourse RemoveRedundantCopy"; }

  bool runOnMachineFunction(MachineFunction &MF) override {
    bool Changed = false;
    for (auto &MBB : MF) {
      for (auto MII = MBB.begin(), MIE = MBB.end(); MII != MIE; ) {
        MachineInstr &MI = *MII++;
        if (MI.isCopy()) {
          Register Dst = MI.getOperand(0).getReg();
          Register Src = MI.getOperand(1).getReg();
          if (Dst == Src) {
            MI.eraseFromParent();
            Changed = true;
          }
        }
      }
    }
    return Changed;
  }
};
} // namespace

char RemoveRedundantCopy::ID = 0;

// Legacy registration for llc -run-pass
static RegisterPass<RemoveRedundantCopy> X("cc-remove-copy", "Remove redundant COPY", false, false);

// New pass manager hook (MIR pipeline)
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCompilerCourseBackendPasses() {}
