#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class ExamplePass : public MachineFunctionPass {
public:
  static char ID;
  ExamplePass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char ExamplePass::ID = 0;

bool ExamplePass::runOnMachineFunction(MachineFunction &func) {
  llvm::outs() << func.getName() << '\n';
  return true;
}
} // namespace

static RegisterPass<ExamplePass> X("CombinationPass", "replaces sequences of logical operations with more efficient instructions", false,
                                   false);
