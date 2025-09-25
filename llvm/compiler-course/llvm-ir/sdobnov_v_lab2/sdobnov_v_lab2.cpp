#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class DivisionOptimizer : public PassInfoMixin<DivisionOptimizer> {
private:
  bool processDivisionInstruction(BinaryOperator *DivisionOp) {
    auto *DivisorConstant = dyn_cast<ConstantInt>(DivisionOp->getOperand(1));
    if (!DivisorConstant)
      return false;

    if (DivisionOp->getOpcode() == Instruction::SDiv) {
      return optimizeSignedDivision(DivisionOp, DivisorConstant);
    } else if (DivisionOp->getOpcode() == Instruction::UDiv) {
      return optimizeUnsignedDivision(DivisionOp, DivisorConstant);
    }
    return false;
  }

  bool optimizeSignedDivision(BinaryOperator *SignedDiv, ConstantInt *Divisor) {
    int64_t divisorValue = Divisor->getSExtValue();
    uint64_t absoluteDivisor = std::abs(divisorValue);

    if (absoluteDivisor == 0 || !isPowerOf2_64(absoluteDivisor)) {
      return false;
    }

    unsigned shiftAmount = Log2_64(absoluteDivisor);
    return replaceDivisionWithShift(SignedDiv, shiftAmount, divisorValue < 0);
  }

  bool optimizeUnsignedDivision(BinaryOperator *UnsignedDiv,
                                ConstantInt *Divisor) {
    uint64_t divisorValue = Divisor->getZExtValue();

    if (divisorValue == 0 || !isPowerOf2_64(divisorValue)) {
      return false;
    }

    unsigned shiftAmount = Log2_64(divisorValue);
    return replaceDivisionWithShift(UnsignedDiv, shiftAmount, false);
  }

  bool replaceDivisionWithShift(BinaryOperator *DivisionOp, unsigned shiftBits,
                                bool needsNegation) {
    IRBuilder<> builder(DivisionOp);
    Value *dividend = DivisionOp->getOperand(0);
    Value *shiftValue = ConstantInt::get(dividend->getType(), shiftBits);

    Value *optimizedResult = createShiftOperation(builder, dividend, shiftValue,
                                                  DivisionOp->getOpcode());

    if (needsNegation) {
      optimizedResult = builder.CreateNeg(optimizedResult, "negated_shift");
    }

    DivisionOp->replaceAllUsesWith(optimizedResult);
    DivisionOp->eraseFromParent();
    return true;
  }

  Value *createShiftOperation(IRBuilder<> &builder, Value *value, Value *shift,
                              Instruction::BinaryOps originalOp) {
    if (originalOp == Instruction::SDiv) {
      return builder.CreateAShr(value, shift, "signed_shift_div");
    } else {
      return builder.CreateLShr(value, shift, "unsigned_shift_div");
    }
  }

public:
  PreservedAnalyses run(Function &func, FunctionAnalysisManager &) {
    bool modificationsMade = false;

    for (auto &basicBlock : func) {
      for (auto instructionIter = basicBlock.begin();
           instructionIter != basicBlock.end();) {
        Instruction *currentInst = &*instructionIter++;

        if (auto *binaryOp = dyn_cast<BinaryOperator>(currentInst)) {
          if (binaryOp->getOpcode() == Instruction::SDiv ||
              binaryOp->getOpcode() == Instruction::UDiv) {
            modificationsMade |= processDivisionInstruction(binaryOp);
          }
        }
      }
    }

    return modificationsMade ? PreservedAnalyses::none()
                             : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivisionOptimizer", "1.0",
          [](llvm::PassBuilder &passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](llvm::StringRef passName,
                   llvm::FunctionPassManager &functionPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (passName == "optimize-divisions") {
                    functionPM.addPass(DivisionOptimizer{});
                    return true;
                  }
                  return false;
                });
          }};
}