#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

bool replaceDivWithShift(Function &F) {
  bool modified = false;

  for (auto &BB : F) {
    for (auto &inst : llvm::make_early_inc_range(BB)) {
      auto *divInst = dyn_cast<BinaryOperator>(&inst);
      if (!divInst)
        continue;

      unsigned opcode = divInst->getOpcode();
      if (opcode != Instruction::SDiv && opcode != Instruction::UDiv)
        continue;

      auto *constDivisor = dyn_cast<ConstantInt>(divInst->getOperand(1));
      if (!constDivisor)
        continue;

      const APInt &divisorValue = constDivisor->getValue();
      if (divisorValue.isZero())
        continue;

      IRBuilder<> builder(divInst);
      Value *dividend = divInst->getOperand(0);
      Value *replacement = nullptr;

      if (divisorValue.isOne()) {
        replacement = dividend;
      } else if (divisorValue.isAllOnes() && opcode == Instruction::SDiv) {
        replacement = builder.CreateNeg(dividend);
      } else if (divisorValue.abs().isPowerOf2()) {
        if (opcode == Instruction::UDiv && divisorValue.isNegative())
          continue;

        uint64_t shiftAmount = divisorValue.abs().exactLogBase2();
        Value *shift = nullptr;

        if (opcode == Instruction::SDiv) {
          shift = builder.CreateAShr(
              dividend, ConstantInt::get(constDivisor->getType(), shiftAmount));
        } else {
          shift = builder.CreateLShr(
              dividend, ConstantInt::get(constDivisor->getType(), shiftAmount));
        }

        if (divisorValue.isNegative()) {
          replacement = builder.CreateNeg(shift);
        } else {
          replacement = shift;
        }
      }

      if (replacement) {
        divInst->replaceAllUsesWith(replacement);
        divInst->eraseFromParent();
        modified = true;
      }
    }
  }

  return modified;
}

struct DivToShiftPass : PassInfoMixin<DivToShiftPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool changed = replaceDivWithShift(F);
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION,
          "DivToShiftPass_Korobeinikov_Arseny_FIIT1_LLVM_IR", "1.0",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (name == "div-to-shift") {
                    FPM.addPass(DivToShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}
