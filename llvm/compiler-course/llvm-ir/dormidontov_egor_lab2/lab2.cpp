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
              int64_t Divisor = C->getSExtValue();

              if (Div->getOpcode() == Instruction::SDiv) {
                int64_t AbsDiv = std::abs(Divisor);
                if (AbsDiv > 0 && isPowerOf2_64(AbsDiv)) {
                  unsigned ShiftAmt = Log2_64(AbsDiv);
                  IRBuilder<> Builder(Div);

                  Value *LHS = Div->getOperand(0);
                  Value *ShiftValue =
                      ConstantInt::get(LHS->getType(), ShiftAmt);
                  Value *Shifted =
                      Builder.CreateAShr(LHS, ShiftValue, "div2shift");

                  Value *NewInstr = Shifted;
                  if (Divisor < 0) {
                    NewInstr = Builder.CreateSub(
                        ConstantInt::get(LHS->getType(), 0), Shifted, "neg");
                  }

                  Div->replaceAllUsesWith(NewInstr);
                  Div->eraseFromParent();
                  Changed = true;
                }
              } else {
                uint64_t UDivisor = C->getZExtValue();
                if (UDivisor > 0 && isPowerOf2_64(UDivisor)) {
                  unsigned ShiftAmt = Log2_64(UDivisor);
                  IRBuilder<> Builder(Div);

                  Value *LHS = Div->getOperand(0);
                  Value *ShiftValue =
                      ConstantInt::get(LHS->getType(), ShiftAmt);

                  Value *NewInstr =
                      Builder.CreateLShr(LHS, ShiftValue, "div2shift");

                  Div->replaceAllUsesWith(NewInstr);
                  Div->eraseFromParent();
                  Changed = true;
                }
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
