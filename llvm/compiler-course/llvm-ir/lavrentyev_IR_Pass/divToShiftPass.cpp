#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

namespace {

struct DivToShiftPass : llvm::PassInfoMixin<DivToShiftPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    llvm::SmallVector<llvm::Instruction *, 8> ToErase;

    for (auto &BB : F) {
      for (auto &I : BB) {
        auto *BinOp = llvm::dyn_cast<llvm::BinaryOperator>(&I);
        if (!BinOp)
          continue;

        auto Op = BinOp->getOpcode();
        if (Op != llvm::Instruction::SDiv && Op != llvm::Instruction::UDiv)
          continue;

        llvm::Value *Dividend = BinOp->getOperand(0);
        llvm::Value *DivisorVal = BinOp->getOperand(1);
        auto *CI = llvm::dyn_cast<llvm::ConstantInt>(DivisorVal);
        if (!CI)
          continue;

        int64_t DivVal = CI->getSExtValue();
        if (DivVal == 0)
          continue;

        llvm::IRBuilder<> Builder(BinOp);
        llvm::Value *Replacement = nullptr;

        if (DivVal == 1) {
          Replacement = Dividend;
        } else if (DivVal == -1 && Op == llvm::Instruction::SDiv) {
          Replacement = Builder.CreateNeg(Dividend, "neg");
        } else {
          uint64_t AbsVal = DivVal < 0 ? static_cast<uint64_t>(-DivVal)
                                       : static_cast<uint64_t>(DivVal);
          if (llvm::isPowerOf2_64(AbsVal)) {
            unsigned ShAmt = llvm::Log2_64(AbsVal);
            if (Op == llvm::Instruction::SDiv) {
              Replacement = Builder.CreateAShr(
                  Dividend, llvm::ConstantInt::get(CI->getType(), ShAmt),
                  "ashr");
              if (DivVal < 0)
                Replacement = Builder.CreateNeg(Replacement, "neg");
            } else {
              Replacement = Builder.CreateLShr(
                  Dividend, llvm::ConstantInt::get(CI->getType(), ShAmt),
                  "lshr");
            }
          }
        }

        if (Replacement) {
          BinOp->replaceAllUsesWith(Replacement);
          ToErase.push_back(BinOp);
          Changed = true;
        }
      }
    }

    for (auto *Inst : ToErase)
      Inst->eraseFromParent();

    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivToShiftCombinedPass", "1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "div-to-shift-combined") {
                    FPM.addPass(DivToShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}
