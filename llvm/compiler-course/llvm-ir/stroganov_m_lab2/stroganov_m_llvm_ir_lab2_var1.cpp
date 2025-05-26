#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FmuladdPass : llvm::PassInfoMixin<FmuladdPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    for (auto &BB : F) {
      for (auto &I : make_early_inc_range(BB)) {
        if (auto *AddOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          if (AddOp->getOpcode() != llvm::Instruction::FAdd)
            continue;
          for (unsigned i = 0; i < 2; ++i) {
            if (auto *MultiplyOp = llvm::dyn_cast<llvm::BinaryOperator>(
                    AddOp->getOperand(i))) {
              if (MultiplyOp->getOpcode() == llvm::Instruction::FMul &&
                  MultiplyOp->hasOneUse()) {
                llvm::IRBuilder<> Builder(AddOp);
                llvm::Value *FMA = Builder.CreateIntrinsic(
                    llvm::Intrinsic::fmuladd, {MultiplyOp->getType()},
                    {MultiplyOp->getOperand(0), MultiplyOp->getOperand(1),
                     AddOp->getOperand(1 - i)},
                    nullptr, "fma");
                AddOp->replaceAllUsesWith(FMA);
                AddOp->eraseFromParent();
                if (MultiplyOp->use_empty())
                  MultiplyOp->eraseFromParent();
                Changed = true;
                break;
              }
            }
          }
        }
      }
    }
    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FmuladdPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "FmuladdPass") {
                    FPM.addPass(FmuladdPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
