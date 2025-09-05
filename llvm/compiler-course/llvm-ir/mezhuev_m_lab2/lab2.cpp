#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct IntegerDivisionOptimizer : PassInfoMixin<IntegerDivisionOptimizer> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Modified = false;

    for (auto &Block : F) {
      for (auto InstIter = Block.begin(); InstIter != Block.end();) {
        Instruction *CurrentInst = &*InstIter++;

        if (auto *DivisionOp = dyn_cast<BinaryOperator>(CurrentInst)) {
          if (DivisionOp->getOpcode() == Instruction::SDiv) {
            Modified |= processSignedDivision(DivisionOp);
          } else if (DivisionOp->getOpcode() == Instruction::UDiv) {
            Modified |= processUnsignedDivision(DivisionOp);
          }
        }
      }
    }

    return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  bool processSignedDivision(BinaryOperator *DivOp) {
    Value *DivisorValue = DivOp->getOperand(1);

    if (auto *ConstDivisor = dyn_cast<ConstantInt>(DivisorValue)) {
      int64_t Divisor = ConstDivisor->getSExtValue();

      if (Divisor == 1 || Divisor == -1) {
        return false;
      }

      if (Divisor > 0 && isPowerOfTwo(Divisor)) {
        return replaceDivisionWithShift(DivOp, Divisor, true);
      }

      if (Divisor < 0 && isPowerOfTwo(-Divisor)) {
        return replaceDivisionWithShift(DivOp, -Divisor, false);
      }
    }

    return false;
  }

  bool processUnsignedDivision(BinaryOperator *DivOp) {
    Value *DivisorValue = DivOp->getOperand(1);

    if (auto *ConstDivisor = dyn_cast<ConstantInt>(DivisorValue)) {
      uint64_t Divisor = ConstDivisor->getZExtValue();

      if (Divisor == 1) {
        return false;
      }

      if (Divisor > 0 && isPowerOfTwo(Divisor)) {
        return replaceDivisionWithShift(DivOp, Divisor, true);
      }
    }

    return false;
  }

  bool isPowerOfTwo(int64_t Value) {
    return Value > 0 && (Value & (Value - 1)) == 0;
  }

  bool replaceDivisionWithShift(BinaryOperator *DivOp, uint64_t PowerOfTwo,
                                bool IsPositive) {
    unsigned ShiftCount = 0;
    uint64_t Temp = PowerOfTwo;
    while (Temp > 1) {
      Temp >>= 1;
      ShiftCount++;
    }

    IRBuilder<> Builder(DivOp);
    Value *Dividend = DivOp->getOperand(0);

    Value *ShiftResult;
    if (DivOp->getOpcode() == Instruction::SDiv) {
      ShiftResult = Builder.CreateAShr(Dividend, ShiftCount, "opt_shift");

      if (!IsPositive) {
        ShiftResult = Builder.CreateNeg(ShiftResult, "opt_neg");
      }
    } else {
      ShiftResult = Builder.CreateLShr(Dividend, ShiftCount, "opt_shift");
    }

    DivOp->replaceAllUsesWith(ShiftResult);
    DivOp->eraseFromParent();
    
    return true;
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IntegerDivisionOptimizer", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "int-div-optimize") {
                    FPM.addPass(DivToBitwiseShiftPass{});
                    return true;
                  }
                  return false;
                });
          }};
}