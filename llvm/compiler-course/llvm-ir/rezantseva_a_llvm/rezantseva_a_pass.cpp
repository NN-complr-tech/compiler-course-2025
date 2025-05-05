#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
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
        auto *Div = llvm::dyn_cast<llvm::BinaryOperator>(&I);
        if (!Div || (Div->getOpcode() != llvm::Instruction::SDiv &&
                     Div->getOpcode() != llvm::Instruction::UDiv)) {
          continue;
        }

        auto *CI = llvm::dyn_cast<llvm::ConstantInt>(Div->getOperand(1));
        if (!CI) {
          continue;
        }

        int64_t Divisor = CI->getSExtValue();
        if (Divisor == 0) {
          continue;
        }

        llvm::IRBuilder<> Builder(Div);
        llvm::Value *Dividend = Div->getOperand(0);

        if (Divisor == 1 || Divisor == -1) {
          llvm::Value *Replacement =
              (Divisor == 1) ? Dividend : Builder.CreateNeg(Dividend);
          Div->replaceAllUsesWith(Replacement);
          ToErase.push_back(Div);
          Changed = true;
          continue;
        }

        if (llvm::isPowerOf2_64(std::abs(Divisor))) {
          unsigned ShiftAmount = llvm::Log2_64(std::abs(Divisor));
          llvm::Value *Shifted;

          if (Div->getOpcode() == llvm::Instruction::SDiv) {
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

    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivToShiftPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "div-to-shift") {
                    FPM.addPass(DivToShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}
