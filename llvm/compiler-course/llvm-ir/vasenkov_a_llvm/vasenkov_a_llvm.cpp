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

class DivisionOptimizer : public PassInfoMixin<DivisionOptimizer> {
private:
  bool isPowerOfTwo(int64_t value) {
    return value > 0 && (value & (value - 1)) == 0;
  }

  unsigned calculateLog2(int64_t value) {
    return Log2_64(static_cast<uint64_t>(value));
  }

public:
  PreservedAnalyses run(Function &func, FunctionAnalysisManager &) {
    bool modified = false;

    for (auto &block : func) {
      SmallVector<Instruction *, 8> instructionsToRemove;

      for (auto &instruction : block) {
        auto *divisionOp = dyn_cast<BinaryOperator>(&instruction);
        if (!divisionOp)
          continue;

        auto opcode = divisionOp->getOpcode();
        if (opcode != Instruction::SDiv && opcode != Instruction::UDiv)
          continue;

        auto *divisorConst = dyn_cast<ConstantInt>(divisionOp->getOperand(1));
        if (!divisorConst)
          continue;

        int64_t divisorValue = divisorConst->getSExtValue();
        if (divisorValue == 0)
          continue;

        IRBuilder<> builder(divisionOp);
        Value *dividend = divisionOp->getOperand(0);
        Type *operandType = dividend->getType();

        if (divisorValue == 1 || divisorValue == -1) {
          Value *optimizedValue =
              (divisorValue == 1) ? dividend : builder.CreateNeg(dividend);

          divisionOp->replaceAllUsesWith(optimizedValue);
          instructionsToRemove.push_back(divisionOp);
          modified = true;
          continue;
        }

        int64_t absDivisor = std::abs(divisorValue);
        if (isPowerOfTwo(absDivisor)) {
          unsigned shiftBits = calculateLog2(absDivisor);
          auto *shiftAmount = ConstantInt::get(operandType, shiftBits);

          Value *shiftedValue = (opcode == Instruction::SDiv)
                                    ? builder.CreateAShr(dividend, shiftAmount)
                                    : builder.CreateLShr(dividend, shiftAmount);

          if (divisorValue < 0 && opcode == Instruction::SDiv) {
            shiftedValue = builder.CreateNeg(shiftedValue);
          }

          divisionOp->replaceAllUsesWith(shiftedValue);
          instructionsToRemove.push_back(divisionOp);
          modified = true;
        }
      }
      for (auto *inst : instructionsToRemove) {
        inst->eraseFromParent();
      }
    }

    return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

} // namespace

extern "C" PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivisionOptimizer", "1.0",
          [](PassBuilder &builder) {
            builder.registerPipelineParsingCallback(
                [](StringRef passName, FunctionPassManager &manager,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (passName == "div-optimize") {
                    manager.addPass(DivisionOptimizer());
                    return true;
                  }
                  return false;
                });
          }};
}