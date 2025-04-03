#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class Fmul_fadd_pass : public llvm::PassInfoMixin<Fmul_fadd_pass> {
private:
  bool wasModified = false;

  void HandleFAddOperation(llvm::BinaryOperator *additionOp) {
    for (int i = 0; i < 2; ++i) {
      if (auto *multiplicationOp=
              llvm::dyn_cast<llvm::BinaryOperator>(additionOp->getOperand(i));
          multiplicationOp&& mul_op->getOpcode() == llvm::Instruction::FMul) {
        TransformToFMAIntrinsic(additionOp, mul_op);
        return;
      }
    }
  }

  void TransformToFMAIntrinsic(llvm::BinaryOperator *addition, llvm::BinaryOperator *multiplication) {
    llvm::IRBuilder<> builder(addition);
    llvm::Value *remainingOperand =
        addition->getOperand(0) == multiplication ? addition->getOperand(1) : addition->getOperand(0);

    addition->replaceAllUsesWith(builder.CreateIntrinsic(
        llvm::Intrinsic::fmuladd, {multiplication->getType()},
        {multiplication->getOperand(0), multiplication->getOperand(1), remainingOperand}));

    addition->eraseFromParent();
    if (multiplication->use_empty()) {
      multiplication->eraseFromParent();
    }
    wasModified = true;
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    for (auto &block : func) {
      for (auto &inst : llvm::make_early_inc_range(block)) {
        if (auto *binaryOp = llvm::dyn_cast<llvm::BinaryOperator>(&inst);
            binaryOp && binaryOp->getOpcode() == llvm::Instruction::FAdd) {
          HandleFAddOperation(binaryOp);
        }
      }
    }

    return wasModified ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

} // namespace

llvm::PassPluginLibraryInfo GetPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Fmul_fadd_pass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "Fmul_fadd_pass") {
                    FPM.addPass(Fmul_fadd_pass{});
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
