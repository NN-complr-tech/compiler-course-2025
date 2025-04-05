#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct DivisionToShiftPass : llvm::PassInfoMixin<DivisionToShiftPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool changed = false;
    for (auto &BB : F) {
      llvm::SmallVector<llvm::Instruction *, 8> toReplace;
      for (auto &I : BB) {
        auto *BO = llvm::dyn_cast<llvm::BinaryOperator>(&I);
        if (!BO || (BO->getOpcode() != llvm::Instruction::SDiv &&
                    BO->getOpcode() != llvm::Instruction::UDiv)) {
          continue;
        }
        auto *C = llvm::dyn_cast<llvm::ConstantInt>(BO->getOperand(1));
        if (!C) {
          continue;
        }
        int64_t divisor = C->getSExtValue();
        if (divisor == 1) {
          llvm::IRBuilder<> Builder(BO);
          llvm::Value *Result = Builder.CreateAdd(
              BO->getOperand(0), llvm::ConstantInt::get(C->getType(), 0));
          BO->replaceAllUsesWith(Result);
          toReplace.push_back(BO);
          changed = true;
          continue;
        }
        if (llvm::isPowerOf2_64(std::abs(divisor))) {
          unsigned shift = llvm::Log2_64(std::abs(divisor));
          llvm::IRBuilder<> Builder(BO);
          llvm::Value *ShiftResult;
          if (BO->getOpcode() == llvm::Instruction::SDiv) {
            ShiftResult = Builder.CreateAShr(
                BO->getOperand(0), llvm::ConstantInt::get(C->getType(), shift));
          } else {
            ShiftResult = Builder.CreateLShr(
                BO->getOperand(0), llvm::ConstantInt::get(C->getType(), shift));
          }
          if (divisor < 0) {
            ShiftResult = Builder.CreateSub(
                llvm::ConstantInt::get(C->getType(), 0), ShiftResult);
          }
          BO->replaceAllUsesWith(ShiftResult);
          toReplace.push_back(BO);
          changed = true;
        }
      }

      for (auto *I : toReplace) {
        I->eraseFromParent();
      }
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getDivToShiftPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivisionToShiftPass", "v1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
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