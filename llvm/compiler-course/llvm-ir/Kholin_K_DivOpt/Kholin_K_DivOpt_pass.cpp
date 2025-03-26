#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

void visitor(llvm::Function &F) {
  for (auto &BB : F) { // function cycle block//
    llvm::SmallVector<llvm::BinaryOperator *, 8> DivsToReplace;
    for (auto &I : BB) { // instruction cycle block
      if (auto *BinOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
        if (BinOp->getOpcode() == llvm::Instruction::SDiv ||
            BinOp->getOpcode() == llvm::Instruction::UDiv) {
          llvm::Value *Divisor = BinOp->getOperand(
              1); // get right operand from result = lhs op rhs
          if (llvm::ConstantInt *ConstantDivisor = llvm::dyn_cast<llvm::ConstantInt>(Divisor)) {
            if (ConstantDivisor->getValue() == 0) { // value of rhs
              continue;
            }
            if (ConstantDivisor->getValue().isPowerOf2()) { // check degree 2
              if (BinOp->getOpcode() == llvm::Instruction::SDiv &&
                  ConstantDivisor->isNegative()) { // negative case
                continue;
              }
              DivsToReplace.push_back(BinOp);
            }
          }
        }
      }
    }

    for (auto *BinOp : DivsToReplace) { // replace div with shift
      uint32_t ShiftAmount =
          llvm::cast<llvm::ConstantInt>(BinOp->getOperand(1))->getValue().exactLogBase2();

      llvm::IRBuilder<> Builder(BinOp); // initialize builder

      llvm::Value *Shift = nullptr;

      if (BinOp->getOpcode() == llvm::Instruction::SDiv) { // signed
        Shift = Builder.CreateAShr(BinOp->getOperand(0), ShiftAmount);
      } else { // unsigned
        Shift = Builder.CreateLShr(BinOp->getOperand(0), ShiftAmount);
      }

      BinOp->replaceAllUsesWith(Shift); // replace

      BinOp->eraseFromParent(); // remove old
    }
  }
}

struct DivOptPass : llvm::PassInfoMixin<DivOptPass> {
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &) {
    visitor(F);
    return llvm::PreservedAnalyses::all(); // preserve all passes
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getDivToShiftPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivOptPass", LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "div-opt-pass") {
                    FPM.addPass(DivOptPass());
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
