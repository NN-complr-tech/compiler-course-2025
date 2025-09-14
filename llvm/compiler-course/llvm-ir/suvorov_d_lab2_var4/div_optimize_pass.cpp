#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

bool optimizeDivision(llvm::Function &F) {
  bool Changed = false;

  for (auto &BB : F) {
    llvm::SmallVector<llvm::Instruction *, 8> ToErase;

    for (auto &I : BB) {
      auto *BinOp = llvm::dyn_cast<llvm::BinaryOperator>(&I);
      if (!BinOp || (BinOp->getOpcode() != llvm::Instruction::SDiv &&
                     BinOp->getOpcode() != llvm::Instruction::UDiv))
        continue;

      auto *Const = llvm::dyn_cast<llvm::ConstantInt>(BinOp->getOperand(1));
      if (!Const)
        continue;

      const llvm::APInt &DivisorValue = Const->getValue();

      if (DivisorValue.isZero())
        continue;

      llvm::IRBuilder<> Builder(BinOp);
      llvm::Value *Replacement = nullptr;

      if (DivisorValue.isOne()) {
        Replacement = BinOp->getOperand(0);
      } else if (DivisorValue.isAllOnes()) {
        Replacement = Builder.CreateNeg(BinOp->getOperand(0));
      } else if (DivisorValue.abs().isPowerOf2()) {
        uint64_t ShiftAmount = DivisorValue.abs().exactLogBase2();

        llvm::Value *Shift = nullptr;

        if (BinOp->getOpcode() == llvm::Instruction::SDiv)
          Shift = Builder.CreateAShr(
              BinOp->getOperand(0),
              llvm::ConstantInt::get(Const->getType(), ShiftAmount));
        else
          Shift = Builder.CreateLShr(
              BinOp->getOperand(0),
              llvm::ConstantInt::get(Const->getType(), ShiftAmount));

        if (DivisorValue.isNegative())
          Replacement = Builder.CreateNeg(Shift);
        else
          Replacement = Shift;
      }

      if (Replacement) {
        BinOp->replaceAllUsesWith(Replacement);
        ToErase.push_back(BinOp);
        Changed = true;
      }
    }

    for (auto *I : ToErase)
      I->eraseFromParent();
  }

  return Changed;
}

struct DivOptimizePass : llvm::PassInfoMixin<DivOptimizePass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = optimizeDivision(F);
    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getDivOptimizePluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivOptimizePass", LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "div-optimize") {
                    FPM.addPass(DivOptimizePass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDivOptimizePluginInfo();
}
