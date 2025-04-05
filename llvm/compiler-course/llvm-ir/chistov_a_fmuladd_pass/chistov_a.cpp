#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class FmuladdPass : public llvm::PassInfoMixin<FmuladdPass> {
private:
  bool changed = false;

  void ProcessFAdd(llvm::BinaryOperator *add_op) {
    for (int i = 0; i < 2; ++i) {
      if (auto *mul_op =
              llvm::dyn_cast<llvm::BinaryOperator>(add_op->getOperand(i));
          mul_op && mul_op->getOpcode() == llvm::Instruction::FMul) {
        ReplaceWithFMA(add_op, mul_op);
        return;
      }
    }
  }

  void ReplaceWithFMA(llvm::BinaryOperator *add, llvm::BinaryOperator *mul) {
    llvm::IRBuilder<> irb(add);
    llvm::Value *other_op =
        add->getOperand(0) == mul ? add->getOperand(1) : add->getOperand(0);

    add->replaceAllUsesWith(irb.CreateIntrinsic(
        llvm::Intrinsic::fmuladd, {mul->getType()},
        {mul->getOperand(0), mul->getOperand(1), other_op}));

    add->eraseFromParent();
    if (mul->use_empty()) {
      mul->eraseFromParent();
    }
    changed = true;
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &function,
                              llvm::FunctionAnalysisManager &) {
    for (auto &basic_block : function) {
      for (auto &instruction : llvm::make_early_inc_range(basic_block)) {
        if (auto *bin_op = llvm::dyn_cast<llvm::BinaryOperator>(&instruction);
            bin_op && bin_op->getOpcode() == llvm::Instruction::FAdd) {
          ProcessFAdd(bin_op);
        }
      }
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

} // namespace

llvm::PassPluginLibraryInfo GetPluginInfo() {
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

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return GetPluginInfo();
}
