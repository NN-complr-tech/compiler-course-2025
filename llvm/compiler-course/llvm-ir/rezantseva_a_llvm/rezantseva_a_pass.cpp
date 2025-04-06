#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct DivToShiftPass : PassInfoMixin<DivToShiftPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Changed = false;
    SmallVector<Instruction *, 8> ToErase;

    for (auto &BB : F) {
      for (auto &I : BB) {
        auto *Div = dyn_cast<BinaryOperator>(&I);
        if (!Div || (Div->getOpcode() != Instruction::SDiv &&
                     Div->getOpcode() != Instruction::UDiv)) {
          continue;
        }

        auto *CI = dyn_cast<ConstantInt>(Div->getOperand(1));
        if (!CI) {
          continue;
        }

        int64_t Divisor = CI->getSExtValue();
        if (Divisor == 0) {
          continue;
        }

        IRBuilder<> Builder(Div);
        Value *Dividend = Div->getOperand(0);

        if (Divisor == 1 || Divisor == -1) {
          Value *Replacement =
              (Divisor == 1) ? Dividend : Builder.CreateNeg(Dividend);
          Div->replaceAllUsesWith(Replacement);
          ToErase.push_back(Div);
          Changed = true;
          continue;
        }

        if (isPowerOf2_64(std::abs(Divisor))) {
          unsigned ShiftAmount = Log2_64(std::abs(Divisor));
          Value *Shifted;

          if (Div->getOpcode() == Instruction::SDiv) {
            Shifted = Builder.CreateAShr(Dividend, ShiftAmount);
            if (Divisor < 0) {
              Shifted = Builder.CreateNeg(Shifted);
            }
          } else {
            Shifted = Builder.CreateLShr(Dividend, ShiftAmount);
          }

          Div->replaceAllUsesWith(Shifted);
          ToErase.push_back(Div);
          Changed = true;
        }
      }
    }

    for (auto *I : ToErase) {
      I->eraseFromParent();
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivToShiftPass", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "div-to-shift") {
                    FPM.addPass(DivToShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}
