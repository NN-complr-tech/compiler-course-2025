#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct DivToBitwiseShiftPass : PassInfoMixin<DivToBitwiseShiftPass> {
  PreservedAnalyses run(Function &Func, FunctionAnalysisManager &) {
    bool Changed = false;

    for (auto &BB : Func) {
      for (auto It = BB.begin(); It != BB.end();) {
        Instruction *Inst = &*It++;
        if (auto *Div = dyn_cast<BinaryOperator>(Inst)) {
          if (Div->getOpcode() == Instruction::SDiv ||
              Div->getOpcode() == Instruction::UDiv) {

            if (auto *C = dyn_cast<ConstantInt>(Div->getOperand(1))) {
              uint64_t Divisor = C->getZExtValue();
              if (isPowerOf2_64(Divisor)) {
                unsigned ShiftAmt = Log2_64(Divisor);
                IRBuilder<> Builder(Div);

                Value *LHS = Div->getOperand(0);
                Value *ShiftValue = ConstantInt::get(LHS->getType(), ShiftAmt);

                Value *NewInstr = nullptr;
                if (Div->getOpcode() == Instruction::SDiv) {
                  NewInstr = Builder.CreateAShr(LHS, ShiftValue, "div2shift");
                } else {
                  NewInstr = Builder.CreateLShr(LHS, ShiftValue, "div2shift");
                }
                Div->replaceAllUsesWith(NewInstr);
                Div->eraseFromParent();
                Changed = true;
              }
            }
          }
        }
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivToBitwiseShiftPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "div-to-shift") {
                    FPM.addPass(DivToBitwiseShiftPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
