#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class ReplaceDivPass : public llvm::PassInfoMixin<ReplaceDivPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool changed = false;

    for (auto &BB : F) {
      llvm::SmallVector<llvm::Instruction *, 8> ToDelete;

      for (auto &I : BB) {
        auto *Div = llvm::dyn_cast<llvm::BinaryOperator>(&I);
        if (!Div)
          continue;

        if (Div->getOpcode() != llvm::Instruction::SDiv &&
            Div->getOpcode() != llvm::Instruction::UDiv)
          continue;

        auto *ConstInt = llvm::dyn_cast<llvm::ConstantInt>(Div->getOperand(1));
        if (!ConstInt)
          continue;

        int64_t divisor = ConstInt->getValue().getSExtValue();
        if (divisor == 0)
          continue;

        llvm::IRBuilder<> Builder(Div);
        llvm::Value *Dividend = Div->getOperand(0);

        if (divisor == 1 || divisor == -1) {
          llvm::Value *NewVal =
              (divisor == 1) ? Dividend : Builder.CreateNeg(Dividend);
          Div->replaceAllUsesWith(NewVal);
          ToDelete.push_back(Div);
          changed = true;
          continue;
        }

        if (llvm::isPowerOf2_64(std::abs(divisor))) {
          unsigned shiftAmount = llvm::Log2_64(std::abs(divisor));
          llvm::Value *Shifted = Builder.CreateAShr(
              Dividend, llvm::ConstantInt::get(Div->getType(), shiftAmount));

          if (divisor < 0) {
            Shifted = Builder.CreateNeg(Shifted);
          }

          Div->replaceAllUsesWith(Shifted);
          ToDelete.push_back(Div);
          changed = true;
        }
      }

      for (auto *I : ToDelete) {
        I->eraseFromParent();
      }
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
};

} // namespace

extern "C" llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ReplaceDivPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "replace-div") {
                    FPM.addPass(ReplaceDivPass());
                    return true;
                  }
                  return false;
                });
          }};
}
