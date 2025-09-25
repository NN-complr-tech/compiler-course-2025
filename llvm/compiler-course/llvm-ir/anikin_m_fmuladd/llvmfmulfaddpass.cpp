#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

struct llvmfmulfaddpass : llvm::PassInfoMixin<llvmfmulfaddpass> {
private:
  bool processFAddInstruction(llvm::Instruction *Inst) {
    auto *addOperation = llvm::dyn_cast<llvm::BinaryOperator>(Inst);
    if (!addOperation || addOperation->getOpcode() != llvm::Instruction::FAdd)
      return false;

    for (unsigned operandIndex = 0; operandIndex < 2; ++operandIndex) {
      llvm::Value *currentOperand = addOperation->getOperand(operandIndex);
      auto *multiplyOperation =
          llvm::dyn_cast<llvm::BinaryOperator>(currentOperand);

      if (multiplyOperation &&
          multiplyOperation->getOpcode() == llvm::Instruction::FMul) {
        if (performFusionTransform(addOperation, multiplyOperation,
                                   operandIndex))
          return true;
      }
    }
    return false;
  }

  bool performFusionTransform(llvm::BinaryOperator *addOp,
                              llvm::BinaryOperator *mulOp, unsigned index) {
    llvm::Value *mulOperand1 = mulOp->getOperand(0);
    llvm::Value *mulOperand2 = mulOp->getOperand(1);
    llvm::Value *addOperand = addOp->getOperand(1 - index);

    if (!verifyTypeConsistency(mulOp, mulOperand1, mulOperand2, addOperand))
      return false;

    return generateFMAInstruction(addOp, mulOp, mulOperand1, mulOperand2,
                                  addOperand);
  }

  bool verifyTypeConsistency(llvm::BinaryOperator *baseOp, llvm::Value *val1,
                             llvm::Value *val2, llvm::Value *val3) {
    llvm::Type *baseType = baseOp->getType();
    return baseType == val1->getType() && baseType == val2->getType() &&
           baseType == val3->getType();
  }

  bool generateFMAInstruction(llvm::BinaryOperator *addOp,
                              llvm::BinaryOperator *mulOp, llvm::Value *a,
                              llvm::Value *b, llvm::Value *c) {
    llvm::IRBuilder<> constructionHelper(addOp);
    llvm::Value *fusedMultiplyAdd = constructionHelper.CreateIntrinsic(
        llvm::Intrinsic::fmuladd, {mulOp->getType()}, {a, b, c});

    fusedMultiplyAdd->takeName(addOp);
    addOp->replaceAllUsesWith(fusedMultiplyAdd);
    addOp->eraseFromParent();

    if (mulOp->use_empty()) {
      mulOp->eraseFromParent();
    }
    return true;
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &function,
                              llvm::FunctionAnalysisManager &analysisManager) {
    bool wasModified = false;

    for (llvm::BasicBlock &basicBlock : function) {
      for (llvm::Instruction &instruction :
           llvm::make_early_inc_range(basicBlock)) {
        if (processFAddInstruction(&instruction)) {
          wasModified = true;
        }
      }
    }

    return wasModified ? llvm::PreservedAnalyses::none()
                       : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "llvmfmulfaddpass", "0.1",
          [](llvm::PassBuilder &passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](llvm::StringRef passName,
                   llvm::FunctionPassManager &functionPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (passName == "llvmfmulfaddpass") {
                    functionPM.addPass(llvmfmulfaddpass{});
                    return true;
                  }
                  return false;
                });
          }};
}
