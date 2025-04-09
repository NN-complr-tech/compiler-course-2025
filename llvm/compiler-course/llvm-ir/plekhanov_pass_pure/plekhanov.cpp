#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {
struct PureFunctionPass : PassInfoMixin<PureFunctionPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    for (auto &BB : F)
      for (auto &I : BB)
        if (I.mayReadOrWriteMemory())
          return PreservedAnalyses::none();

    F.addFnAttr("pure");
    return PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};
} // namespace
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionPass", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "pure-func-pass") {
                    FPM.addPass(PureFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
