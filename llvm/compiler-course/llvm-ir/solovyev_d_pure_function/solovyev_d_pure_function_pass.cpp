#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct PureFunctionPass : llvm::PassInfoMixin<PureFunctionPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    if (!func.hasFnAttribute("pure")) {
      bool isPure = true;
      for (auto &block : func) {
        for (auto &inst : block) {
          if (inst.mayReadOrWriteMemory()) {
            isPure = false;
            break;
          }
        }
        if (!isPure) {
          break;
        }
      }
      if (isPure) {
        func.addFnAttr("pure");
      }
    }
    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "PureFunctionPass") {
                    FPM.addPass(PureFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
