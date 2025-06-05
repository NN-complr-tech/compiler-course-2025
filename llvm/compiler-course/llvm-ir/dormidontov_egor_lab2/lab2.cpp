#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>

using namespace llvm;

namespace {
static bool isPowerOfTwo(uint64_t n) { return n && !(n & (n - 1)); }

static unsigned getLog2(uint64_t n) {
  // n - степень двойки, log2(n) - позиция единственного бита
  unsigned res = 0;
  while (n >>= 1)
    ++res;
  return res;
}

struct DivToBitwiseShiftPass : llvm::PassInfoMixin<DivToBitwiseShiftPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool changed = false;

    for (auto &BB : func) {
      for (auto it = BB.begin(); it != BB.end();) {
        Instruction *inst = &*it++;
        if (auto *div = dyn_cast<BinaryOperator>(inst)) {
          if (div->getOpcode() == Instruction::SDiv ||
              div->getOpcode() == Instruction::UDiv) {

            if (auto *C = dyn_cast<ConstantInt>(div->getOperand(1))) {
              uint64_t divisor = C->getZExtValue();
              if (isPowerOfTwo(divisor)) {
                unsigned shift = getLog2(divisor);
                IRBuilder<> builder(div);

                Value *lhs = div->getOperand(0);
                Value *shift_amt = ConstantInt::get(lhs->getType(), shift);

                Value *newInstr = nullptr;
                if (div->getOpcode() == Instruction::SDiv) {
                  newInstr = builder.CreateAShr(lhs, shift_amt, "div2shift");
                } else {
                  newInstr = builder.CreateLShr(lhs, shift_amt, "div2shift");
                }
                div->replaceAllUsesWith(newInstr);
                div->eraseFromParent();
                changed = true;
              }
            }
          }
        }
      }
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
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
