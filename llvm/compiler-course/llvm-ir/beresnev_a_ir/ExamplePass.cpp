#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct PureFunctionPass : llvm::PassInfoMixin<PureFunctionPass> {

  bool isPureFunction(llvm::Function &F) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *LI = llvm::dyn_cast<llvm::LoadInst>(&I)) {
          if (LI->isVolatile())
            return false;
          llvm::Value *ptr = LI->getPointerOperand();
          if (auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(
                  ptr->stripPointerCasts()))
            return false;
        }
      }
    }
    return true;
  }

  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    if (isPureFunction(F)) {
      F.addFnAttr("pure");
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
                  if (name == "pure_fn") {
                    FPM.addPass(PureFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
