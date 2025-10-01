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

    if (func.isDeclaration())
      return llvm::PreservedAnalyses::all();

    for (auto &bb : func) {
      for (auto &inst : bb) {
        if (llvm::isa<llvm::StoreInst>(&inst))
          return llvm::PreservedAnalyses::all();
        if (llvm::isa<llvm::CallInst>(&inst))
          return llvm::PreservedAnalyses::all();
      }
    }

    func.addFnAttr("pure");
    return llvm::PreservedAnalyses::all();
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  return name == "mark-pure"
                             ? (FPM.addPass(PureFunctionPass{}), true)
                             : false;
                });
          }};
}
