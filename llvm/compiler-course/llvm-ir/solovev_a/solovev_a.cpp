#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class DivToShiftPass : public llvm::PassInfoMixin<DivToShiftPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {
    bool changed = false;
    for (auto &BB : F) {
      for (auto &I : llvm::make_early_inc_range(BB)) {
        if (auto *Div = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          bool isSigned = Div->getOpcode() == llvm::Instruction::SDiv;
          bool isUnsigned = Div->getOpcode() == llvm::Instruction::UDiv;
          if (isSigned || isUnsigned) {
            if (auto *ConstInt =
                    llvm::dyn_cast<llvm::ConstantInt>(Div->getOperand(1))) {
              int64_t divisor = isSigned ? ConstInt->getSExtValue()
                                         : ConstInt->getZExtValue();
              llvm::IRBuilder<> Builder(Div);
              llvm::Value *Dividend = Div->getOperand(0);
              if (divisor == 1 || (isSigned && divisor == -1)) {
                llvm::Value *NewVal =
                    (divisor == 1) ? Dividend
                                   : Builder.CreateNeg(Dividend, "neg_tmp");
                Div->replaceAllUsesWith(NewVal);
                Div->eraseFromParent();
                changed = true;
                continue;
              }
              if (divisor != 0 && (abs(divisor) & (abs(divisor) - 1)) == 0) {
                int shiftAmount = llvm::Log2_64(abs(divisor));
                llvm::Value *Shifted =
                    isSigned
                        ? Builder.CreateAShr(Dividend, shiftAmount, "sh_tmp")
                        : Builder.CreateLShr(Dividend, shiftAmount, "lsh_tmp");
                if (isSigned && divisor < 0) {
                  Shifted = Builder.CreateNeg(Shifted, "neg_tmp");
                }
                Div->replaceAllUsesWith(Shifted);
                Div->eraseFromParent();
                changed = true;
              }
            }
          }
        }
      }
    }
    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

} // namespace

extern "C" llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivToShiftPass", "v1.0",
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
