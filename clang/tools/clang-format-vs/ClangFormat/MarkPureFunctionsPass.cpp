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
    if (F.isDeclaration())
      return PreservedAnalyses::all();

    bool isPure = true;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *Call = dyn_cast<CallBase>(&I)) {
          if (!Call->onlyReadsMemory() && !Call->doesNotAccessMemory()) {
            isPure = false;
            break;
          }
        } else if (I.mayWriteToMemory()) {
          isPure = false;
          break;
        }
      }
      if (!isPure)
        break;
    }

    if (isPure) {
      F.setDoesNotAccessMemory();
      errs() << "Marking function '" << F.getName() << "' as pure (readnone)\n";
    } else {
      errs() << "Function '" << F.getName() << "' is impure\n";
    }

    return PreservedAnalyses::all();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MarkPureFunctionsPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mark-pure-func") {
                    FPM.addPass(MarkPureFunctionsPass());
                    return true;
                  }
                  return false;
                });
          }};
}