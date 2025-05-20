#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct MarkPureFunctionsPass : PassInfoMixin<MarkPureFunctionsPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    if (!F.hasFnAttribute("pure")) {
      bool readsOrWrites = false;

      for (auto &BB : F) {
        for (auto &I : BB) {
          if (I.mayReadOrWriteMemory()) {
            readsOrWrites = true;
            break;
          }
        }
        if (readsOrWrites)
          break;
      }

      if (!readsOrWrites) {
        F.addFnAttr("pure");
      }
    }

    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MarkPureFunctionsPass", "1.0",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mark-pure") {
                    FPM.addPass(MarkPureFunctionsPass{});
                    return true;
                  }
                  return false;
                });
          }};
}