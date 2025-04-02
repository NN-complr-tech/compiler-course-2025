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
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool modified = false;

    for (auto &Block : Func) {
      llvm::SmallVector<llvm::Instruction *, 8> ToErase;

      for (auto &Inst : Block) {
        auto *BinOp = llvm::dyn_cast<llvm::BinaryOperator>(&Inst);
        if (!BinOp)
          continue;

        if (BinOp->getOpcode() != llvm::Instruction::SDiv &&
            BinOp->getOpcode() != llvm::Instruction::UDiv)
          continue;

        auto *ConstVal =
            llvm::dyn_cast<llvm::ConstantInt>(BinOp->getOperand(1));
        if (!ConstVal)
          continue;

        int64_t divisor = ConstVal->getValue().getSExtValue();
        if (divisor == 0)
          continue;

        llvm::IRBuilder<> Builder(BinOp);
        llvm::Value *Dividend = BinOp->getOperand(0);

        if (divisor == 1 || divisor == -1) {
          llvm::Value *NewVal =
              (divisor == 1) ? Dividend : Builder.CreateNeg(Dividend);
          BinOp->replaceAllUsesWith(NewVal);
          ToErase.push_back(BinOp);
          modified = true;
          continue;
        }

        if ((std::abs(divisor) & (std::abs(divisor) - 1)) == 0) {
          unsigned ShiftAmount = llvm::Log2_64(std::abs(divisor));
          llvm::Value *Shifted = Builder.CreateAShr(
              Dividend, llvm::ConstantInt::get(BinOp->getType(), ShiftAmount));

          if (divisor < 0) {
            Shifted = Builder.CreateNeg(Shifted);
          }

          BinOp->replaceAllUsesWith(Shifted);
          ToErase.push_back(BinOp);
          modified = true;
        }
      }

      for (auto *Inst : ToErase) {
        Inst->eraseFromParent();
      }
    }

    return modified ? llvm::PreservedAnalyses::none()
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
