#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class FmuladdPass : public llvm::PassInfoMixin<FmuladdPass> {
 private:
  bool changed = false;

  bool ProcessFAdd(llvm::BinaryOperator *add_op) {
    if (auto *mul_op = llvm::dyn_cast<llvm::BinaryOperator>(add_op->getOperand(0));
        mul_op && mul_op->getOpcode() == llvm::Instruction::FMul) {
      ReplaceWithFMA(add_op, mul_op);
      changed = true;
    }
    return changed;
  }

  void ReplaceWithFMA(llvm::BinaryOperator *add, llvm::BinaryOperator *mul) {
    llvm::IRBuilder<> irb(add);
    auto fma = irb.CreateIntrinsic(llvm::Intrinsic::fmuladd,
                                  {mul->getOperand(0)->getType()},
                                  {mul->getOperand(0), mul->getOperand(1), add->getOperand(1)});

    add->replaceAllUsesWith(fma);
    add->eraseFromParent();
    if (mul->use_empty()) {
      mul->eraseFromParent();
    }
  }

 public:
  llvm::PreservedAnalyses run(llvm::Function &function,
                               llvm::FunctionAnalysisManager &) {
    for (llvm::BasicBlock &basic_block : function) {
      for (auto it = basic_block.begin(); it != basic_block.end();) {
        llvm::Instruction *instruction = &*(it++);

        if (auto *bin_op = llvm::dyn_cast<llvm::BinaryOperator>(instruction);
            bin_op && bin_op->getOpcode() == llvm::Instruction::FAdd) {
          ProcessFAdd(bin_op);
        }
      }
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

}  // namespace

llvm::PassPluginLibraryInfo GetPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FmuladdPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
                    -> bool {
                  if (name == "FmuladdPass") {
                    FPM.addPass(FmuladdPass{});
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return GetPluginInfo();
}
