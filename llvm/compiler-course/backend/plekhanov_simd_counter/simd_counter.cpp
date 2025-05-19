#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class SimdPass : public MachineFunctionPass {
public:
  static char ID;
  SimdPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char SimdPass::ID = 0;

bool SimdPass::runOnMachineFunction(MachineFunction &func) {
  llvm::outs() << func.getName() << '\n';
  return true;
}
} // namespace

static RegisterPass<SimdPass> X("example-x86", "description pass", false,
                                   false);
