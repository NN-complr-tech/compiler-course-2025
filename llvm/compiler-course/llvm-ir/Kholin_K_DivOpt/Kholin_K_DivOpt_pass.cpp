#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

void visitor(Function &F) {
  errs() << "Start processing function: " << F.getName() << "\n";
  for (auto &BB : F) { // function cycle block//
    errs() << ">Start processing basic block" << "\n";
    std::vector<BinaryOperator *> DivsToReplace;
    for (auto &I : BB) { // instruction cycle block
      if (auto *BinOp = dyn_cast<BinaryOperator>(&I)) {
        if (BinOp->getOpcode() == Instruction::SDiv ||
            BinOp->getOpcode() == Instruction::UDiv) {
          Value *Divisor = BinOp->getOperand(
              1); // get right operand from result = lhs op rhs
          if (ConstantInt *ConstantDivisor = dyn_cast<ConstantInt>(Divisor)) {
            if (ConstantDivisor->getValue() == 0) { // value of rhs
              errs() << "Division by zero detected in function " << F.getName()
                     << "\n";
              continue;
            }
            if (ConstantDivisor->getValue().isPowerOf2()) { // check degree 2
              errs() << "Processing div instruction: " << *BinOp << "\n";
              errs() << "Divisor is a power of 2: " << *ConstantDivisor << "\n";
              if (BinOp->getOpcode() == Instruction::SDiv &&
                  ConstantDivisor->isNegative()) { // negative case
                errs() << "Signed division by negative power of 2 detected in "
                          "function "
                       << F.getName() << "\n";
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
          cast<ConstantInt>(BinOp->getOperand(1))->getValue().exactLogBase2();
      errs() << "Shift amount: " << ShiftAmount << "\n";

      IRBuilder<> Builder(BinOp); // initialize builder
      errs() << "Initialize Builder for instruction: " << *BinOp << "\n";

      Value *Shift = nullptr;

      if (BinOp->getOpcode() == Instruction::SDiv) { // signed
        Shift = Builder.CreateAShr(BinOp->getOperand(0), ShiftAmount);
      } else { // unsigned
        Shift = Builder.CreateLShr(BinOp->getOperand(0), ShiftAmount);
      }
      errs() << "Constructed shift instruction: " << *Shift << "\n";

      BinOp->replaceAllUsesWith(Shift); // replace
      errs() << "Division successfully replaced!\n";

      BinOp->eraseFromParent(); // remove old
      errs() << "Div instruction successfully removed!\n";
    }
    errs() << ">End processing basic block" << "\n";
  }
  errs() << "End processing function: " << F.getName() << "\n";
}

struct DivOptPass : PassInfoMixin<DivOptPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    visitor(F);
    return PreservedAnalyses::all(); // preserve all passes
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getDivToShiftPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivOptPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "div-to-shift") {
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
