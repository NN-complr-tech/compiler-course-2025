#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FMAPass : llvm::PassInfoMixin<FMAPass> {
  void createFMA(llvm::BinaryOperator *add, llvm::BinaryOperator *mul, int op) {
    llvm::Value *a = mul->getOperand(0);
    llvm::Value *b = mul->getOperand(1);
    llvm::Value *c = add->getOperand(op);

    llvm::IRBuilder<> irb(add);
    llvm::Value *fma = irb.CreateIntrinsic(llvm::Intrinsic::fmuladd,
                                           {a->getType()}, {a, b, c});

    add->replaceAllUsesWith(fma);
    add->eraseFromParent();
    if (mul->use_empty()) {
      mul->eraseFromParent();
    }
  }

  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    llvm::PreservedAnalyses out = llvm::PreservedAnalyses::all();

    for (llvm::BasicBlock &basic_block : func) {
      for (llvm::Instruction &instruction :
           llvm::make_early_inc_range(basic_block)) {
        if (llvm::BinaryOperator *add =
                llvm::dyn_cast<llvm::BinaryOperator>(&instruction)) {
          if (add->getOpcode() == llvm::Instruction::FAdd) {

            if (llvm::BinaryOperator *mul =
                    llvm::dyn_cast<llvm::BinaryOperator>(add->getOperand(0))) {
              if (mul->getOpcode() == llvm::Instruction::FMul) {
                createFMA(add, mul, 1);
                out = llvm::PreservedAnalyses::none();
              }
            } else if (llvm::BinaryOperator *mul =
                           llvm::dyn_cast<llvm::BinaryOperator>(
                               add->getOperand(1))) {
              if (mul->getOpcode() == llvm::Instruction::FMul) {
                createFMA(add, mul, 0);
                out = llvm::PreservedAnalyses::none();
              }
            }
          }
        }
      }
    }

    return out;
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMAPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "kurakin_fma") {
                    FPM.addPass(FMAPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
