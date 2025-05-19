#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {
struct PureFunctionPass : llvm::PassInfoMixin<PureFunctionPass> {
    llvm::PreservedAnalyses run(llvm::Function &F,
				 llvm::FunctionAnalysisManager &) {
    for (auto &BB : F)
      for (auto &I : BB)
        if (I.mayReadOrWriteMemory())
          return llvm::PreservedAnalyses::none();

    F.addFnAttr("pure");
    return llvm::PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "pure-func-pass") {
                    FPM.addPass(PureFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}