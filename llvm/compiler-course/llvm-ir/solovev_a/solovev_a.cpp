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
          if (Div->getOpcode() == llvm::Instruction::SDiv) {
            if (auto *ConstInt =
                    llvm::dyn_cast<llvm::ConstantInt>(Div->getOperand(1))) {
              int64_t divisor = ConstInt->getSExtValue();
              llvm::IRBuilder<> Builder(Div);
              if (divisor == 1 || divisor == -1) {
                llvm::Value *NewVal;
                if (divisor == 1) {
                  NewVal = Builder.CreateAdd(
                      Div->getOperand(0),
                      llvm::ConstantInt::get(Div->getType(), 0), "add_zero");
                } else {
                  NewVal = Builder.CreateNeg(Div->getOperand(0), "neg_tmp");
                }
                Div->replaceAllUsesWith(NewVal);
                Div->eraseFromParent();
                changed = true;
                continue;
              }
              if (divisor != 0 && (abs(divisor) & (abs(divisor) - 1)) == 0) {
                int shiftAmount = llvm::Log2_64(abs(divisor));
                llvm::Value *Shifted = Builder.CreateAShr(
                    Div->getOperand(0), shiftAmount, "sh_tmp");
                if (divisor < 0) {
                  Shifted = Builder.CreateNeg(Shifted, "neg_tmp");
                }
                Div->replaceAllUsesWith(Shifted);
                Div->eraseFromParent();
                changed = true;
              }
            }
          }
          if (Div->getOpcode() == llvm::Instruction::UDiv) {
            if (auto *ConstInt =
                    llvm::dyn_cast<llvm::ConstantInt>(Div->getOperand(1))) {
              uint64_t divisor = ConstInt->getZExtValue();
              llvm::IRBuilder<> Builder(Div);
              if (divisor == 1) {
                llvm::Value *NewVal = Builder.CreateAdd(
                    Div->getOperand(0),
                    llvm::ConstantInt::get(Div->getType(), 0), "add_zero");
                Div->replaceAllUsesWith(NewVal);
                Div->eraseFromParent();
                changed = true;
                continue;
              }
              if (divisor != 0 && (divisor & (divisor - 1)) == 0) {
                int shiftAmount = llvm::Log2_64(divisor);
                llvm::Value *Shifted = Builder.CreateLShr(
                    Div->getOperand(0), shiftAmount, "lsh_tmp");
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
