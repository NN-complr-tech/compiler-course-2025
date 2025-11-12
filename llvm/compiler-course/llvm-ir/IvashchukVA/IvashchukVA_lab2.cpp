#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct MarkPureFunctionsPass : public PassInfoMixin<MarkPureFunctionsPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    if (F.isDeclaration() || F.isIntrinsic()) {
      return PreservedAnalyses::all();
    }

    if (F.doesNotAccessMemory()) {
      return PreservedAnalyses::all();
    }

    bool IsPure = true;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *Call = dyn_cast<CallBase>(&I)) {
          Function *Callee = Call->getCalledFunction();
          if (Callee && !Callee->isIntrinsic() &&
              !Callee->doesNotAccessMemory()) {
            IsPure = false;
            break;
          }
        }

        if (I.mayWriteToMemory()) {
          IsPure = false;
          break;
        }
      }
      if (!IsPure) {
        break;
      }
    }

    if (IsPure) {
      F.setDoesNotAccessMemory();
    }

    return PreservedAnalyses::all();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionsPass_IvashchukVA_FIIT2_LLVM_IR", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mark-pure-functions") {
                    FPM.addPass(MarkPureFunctionsPass());
                    return true;
                  }
                  return false;
                });
          }};
}
