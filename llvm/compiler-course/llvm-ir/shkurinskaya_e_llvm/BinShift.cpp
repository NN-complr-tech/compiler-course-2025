#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class BinShiftPass : public llvm::PassInfoMixin<BinShiftPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Modified = false;

    for (auto &BB : F) {
      llvm::SmallVector<llvm::Instruction *, 16> WorkList;

      for (auto &I : BB) {
        if (auto *Div = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          if (Div->getOpcode() == llvm::Instruction::SDiv ||
              Div->getOpcode() == llvm::Instruction::UDiv) {
            WorkList.push_back(Div);
          }
        }
      }

      for (auto *DivInst : WorkList) {
        if (processDivision(DivInst)) {
          Modified = true;
        }
      }
    }

    return Modified ? llvm::PreservedAnalyses::none()
                    : llvm::PreservedAnalyses::all();
  }

private:
  bool processDivision(llvm::Instruction *Div) {
    llvm::IRBuilder<> Builder(Div);

    auto *Dividend = Div->getOperand(0);
    auto *DivisorOp = Div->getOperand(1);

    auto IsSigned = Div->getOpcode() == llvm::Instruction::SDiv;

    int64_t DivisorValue = getConstantValue(DivisorOp);

    if (DivisorValue == 1 || DivisorValue == -1) {
      if (DivisorValue == 1) {
        Div->replaceAllUsesWith(Dividend);
      } else {
        auto *NegVal = Builder.CreateNeg(Dividend, "negation_tmp");
        Div->replaceAllUsesWith(NegVal);
      }
      Div->eraseFromParent();
      return true;
    }

    int64_t AbsDivisorValue = std::abs(DivisorValue);

    if (isPowerOfTwo(AbsDivisorValue)) {
      if (DivisorValue < 0) {
        Dividend = Builder.CreateNeg(Dividend, "negation_tmp");
      }
      replaceDivisionWithShift(Div, Dividend, AbsDivisorValue, IsSigned,
                               Builder);
      return true;
    }

    return false; // Non-constant divisors pass through unchanged.
  }

  int64_t getConstantValue(llvm::Value *Val) {
    if (auto *ConstInt = llvm::dyn_cast<llvm::ConstantInt>(Val)) {
      return ConstInt->getSExtValue();
    }

    if (auto *Instr = llvm::dyn_cast<llvm::Instruction>(Val)) {
      if (Instr->getOpcode() == llvm::Instruction::Mul) {
        auto *Op1 = Instr->getOperand(0);
        auto *Op2 = Instr->getOperand(1);

        int64_t Op1Val = getConstantValue(Op1);
        int64_t Op2Val = getConstantValue(Op2);

        if (Op1Val != 0 && Op2Val != 0) {
          return Op1Val * Op2Val;
        }
      }
    }

    return 0; // Return 0 if not a constant value.
  }

  bool isPowerOfTwo(int64_t Value) {
    return Value > 0 && (Value & (Value - 1)) == 0;
  }

  void replaceDivisionWithShift(llvm::Instruction *Div, llvm::Value *Dividend,
                                int64_t DivisorValue, bool IsSigned,
                                llvm::IRBuilder<> &Builder) {
    unsigned ShiftAmount = llvm::Log2_64(DivisorValue);
    llvm::Value *ShiftedValue = nullptr;

    if (IsSigned) {
      ShiftedValue = Builder.CreateAShr(
          Dividend, llvm::ConstantInt::get(Div->getType(), ShiftAmount),
          "signed_shift");
    } else {
      ShiftedValue = Builder.CreateLShr(
          Dividend, llvm::ConstantInt::get(Div->getType(), ShiftAmount),
          "unsigned_shift");
    }

    Div->replaceAllUsesWith(ShiftedValue);
    Div->eraseFromParent();
  }
};

} // namespace

extern "C" llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "BinShiftPass", "1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "bin-shift") {
                    FPM.addPass(BinShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}
