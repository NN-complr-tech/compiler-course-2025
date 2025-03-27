#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

static bool isPowerOfTwo(int64_t n) {
  if (n > 0) {
    return (n & (n - 1)) == 0;
  } else if (n < 0) {
    return isPowerOfTwo(-n);
  }
  return false;
}

static unsigned getLog2(int64_t n) {
  n = n < 0 ? -n : n;
  unsigned log = 0;
  while (n >>= 1)
    ++log;
  return log;
}

struct DivisionToShiftPass : PassInfoMixin<DivisionToShiftPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool changed = false;

    for (auto &BB : F) {
      llvm::SmallVector<Instruction *> toReplace;

      for (auto &I : BB) {
        if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
          if (BO->getOpcode() == Instruction::SDiv ||
              BO->getOpcode() == Instruction::UDiv) {
            if (auto *C = dyn_cast<ConstantInt>(BO->getOperand(1))) {
              int64_t divisor = C->getSExtValue();

              if (divisor == 1) {
                IRBuilder<> Builder(BO);
                Value *Result = Builder.CreateAdd(
                    BO->getOperand(0), ConstantInt::get(C->getType(), 0));
                BO->replaceAllUsesWith(Result);
                toReplace.push_back(BO);
                changed = true;
                continue;
              }

              if (isPowerOfTwo(std::abs(divisor))) {
                unsigned shift = getLog2(std::abs(divisor));
                IRBuilder<> Builder(BO);
                Value *ShiftResult;

                if (BO->getOpcode() == Instruction::SDiv) {
                  ShiftResult = Builder.CreateAShr(
                      BO->getOperand(0), ConstantInt::get(C->getType(), shift));
                } else {
                  ShiftResult = Builder.CreateLShr(
                      BO->getOperand(0), ConstantInt::get(C->getType(), shift));
                }

                if (divisor < 0) {
                  ShiftResult = Builder.CreateSub(
                      ConstantInt::get(C->getType(), 0), ShiftResult);
                }

                BO->replaceAllUsesWith(ShiftResult);
                toReplace.push_back(BO);
                changed = true;
              }
            }
          }
        }
      }

      for (auto *I : toReplace) {
        I->eraseFromParent();
      }
    }

    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

llvm::PassPluginLibraryInfo getDivToShiftPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivisionToShiftPass", "v1.0",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (Name == "div2shift") {
                    FPM.addPass(DivisionToShiftPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getDivToShiftPluginInfo();
}
