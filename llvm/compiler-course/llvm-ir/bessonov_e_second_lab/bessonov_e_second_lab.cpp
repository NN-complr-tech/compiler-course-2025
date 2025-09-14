#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

  struct MarkPureFunctionsPass : PassInfoMixin<MarkPureFunctionsPass> {

    static bool doesFunctionAccessMemory(const Function& F) {
      for (const auto& BB : F) {
        for (const auto& I : BB) {
          if (I.mayReadOrWriteMemory())
            return true;
        }
      }
      return false;
    }

    PreservedAnalyses run(Function& F, FunctionAnalysisManager&) {
      if (!F.hasFnAttribute("pure") && !doesFunctionAccessMemory(F)) {
        F.addFnAttr("pure");
      }

      return PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
  };

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "MarkPureFunctionsPass",
    "1.0",
    [](PassBuilder& PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager& FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "mark-pure") {
            FPM.addPass(MarkPureFunctionsPass{});
            return true;
          }
          return false;
        }
      );
    }
  };
}
