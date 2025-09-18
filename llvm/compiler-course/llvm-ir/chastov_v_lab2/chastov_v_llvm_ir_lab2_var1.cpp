#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {

class FMAOptimizer : public PassInfoMixin<FMAOptimizer> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Modified = false;

    for (auto &BB : F) {
      for (auto &Inst : make_early_inc_range(BB)) {
        if (auto *AddOp = dyn_cast<BinaryOperator>(&Inst)) {
          if (AddOp->getOpcode() == Instruction::FAdd) {
            Modified |= tryFuseMultiplyAdd(AddOp);
          }
        }
      }
    }

    return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  bool tryFuseMultiplyAdd(BinaryOperator *AddOp) {
    for (unsigned i = 0; i < 2; ++i) {
      Value *Operand = AddOp->getOperand(i);
      if (auto *MulOp = dyn_cast<BinaryOperator>(Operand)) {
        if (isValidMultiplyForFusion(MulOp)) {
          return replaceWithFusedOp(AddOp, MulOp, i);
        }
      }
    }
    return false;
  }

  bool isValidMultiplyForFusion(BinaryOperator *MulOp) {
    return MulOp->getOpcode() == Instruction::FMul && MulOp->hasOneUse();
  }

  bool replaceWithFusedOp(BinaryOperator *AddOp, BinaryOperator *MulOp,
                          unsigned MulOperandIdx) {
    IRBuilder<> Builder(AddOp);
    Value *C = AddOp->getOperand(1 - MulOperandIdx);

    Value *FMA = Builder.CreateIntrinsic(
        Intrinsic::fmuladd, {MulOp->getType()},
        {MulOp->getOperand(0), MulOp->getOperand(1), C}, nullptr, "fma");

    AddOp->replaceAllUsesWith(FMA);
    AddOp->eraseFromParent();

    if (MulOp->use_empty()) {
      MulOp->eraseFromParent();
    }

    return true;
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMAOptimizer", "v1.0", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "fma-opt") {
                    FPM.addPass(FMAOptimizer());
                    return true;
                  }
                  return false;
                });
          }};
}
