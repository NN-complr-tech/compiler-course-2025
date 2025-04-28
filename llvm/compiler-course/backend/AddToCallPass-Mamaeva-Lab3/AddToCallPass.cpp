#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "add-to-call"

using namespace llvm;

namespace {

class AddToCallPass : public MachineFunctionPass {
public:
  static char ID;

  AddToCallPass() : MachineFunctionPass(ID) {
    initializeAddToCallPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    const X86Subtarget &ST = MF.getSubtarget<X86Subtarget>();
    const X86InstrInfo *TII = ST.getInstrInfo();
    TRI = ST.getRegisterInfo();
    bool Changed = false;

    // Get the LLVM function and module
    Function &F = MF.getFunction();
    Module *M = F.getParent();

    // Look for 'add' function in the module
    Function *AddFunc = M->getFunction("add");
    if (!AddFunc || AddFunc->isDeclaration()) {
      LLVM_DEBUG(dbgs() << "Add function not found or is declaration\n");
      return false;
    }

    // Verify function signature: i32 (i32, i32)
    if (AddFunc->getFunctionType()->getNumParams() != 2 ||
        !AddFunc->getReturnType()->isIntegerTy(32) ||
        !AddFunc->getFunctionType()->getParamType(0)->isIntegerTy(32) ||
        !AddFunc->getFunctionType()->getParamType(1)->isIntegerTy(32)) {
      LLVM_DEBUG(dbgs() << "Add function has wrong signature\n");
      return false;
    }

    for (auto &MBB : MF) {
      SmallVector<MachineInstr *, 4> ToRemove;

      for (auto &MI : MBB) {
        if (MI.getOpcode() != X86::ADD32rr)
          continue;

        LLVM_DEBUG(dbgs() << "Found ADD32rr instruction: "; MI.dump());

        Register DstReg = MI.getOperand(0).getReg();
        Register Src1Reg = MI.getOperand(1).getReg();
        Register Src2Reg = MI.getOperand(2).getReg();
        DebugLoc DL = MI.getDebugLoc();

        // Create call instruction
        BuildMI(MBB, MI, DL, TII->get(X86::CALL64pcrel32))
            .addGlobalAddress(AddFunc)
            .addReg(Src1Reg, RegState::Implicit)
            .addReg(Src2Reg, RegState::Implicit)
            .addReg(DstReg, RegState::ImplicitDefine)
            .addRegMask(TRI->getCallPreservedMask(MF, F.getCallingConv()));

        ToRemove.push_back(&MI);
        Changed = true;
      }

      // Remove original ADD instructions
      for (auto *MI : ToRemove) {
        MI->eraseFromParent();
      }
    }

    return Changed;
  }

  StringRef getPassName() const override { return "Replace ADD with CALL"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  const TargetRegisterInfo *TRI;
};

char AddToCallPass::ID = 0;

} // end anonymous namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AddToCallPass", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, MachineFunctionPassManager &MFPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "add-to-call") {
                    MFPM.addPass(AddToCallPass());
                    return true;
                  }
                  return false;
                });
          }};
}
