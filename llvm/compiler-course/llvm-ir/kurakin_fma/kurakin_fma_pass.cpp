#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FMAPass : llvm::PassInfoMixin<FMAPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    llvm::PreservedAnalyses out = llvm::PreservedAnalyses::all();

    for (llvm::BasicBlock &basic_block : func) {
      for (auto start = basic_block.begin(), end = basic_block.end();
           start != end;) {
        llvm::Instruction *instruction = &*start;
        ++start;

        if (llvm::BinaryOperator *add =
                llvm::dyn_cast<llvm::BinaryOperator>(instruction)) {
          if (add->getOpcode() == llvm::Instruction::FAdd) {

            if (llvm::BinaryOperator *mul =
                    llvm::dyn_cast<llvm::BinaryOperator>(add->getOperand(0))) {
              if (mul->getOpcode() == llvm::Instruction::FMul) {
                llvm::Value *a = mul->getOperand(0);
                llvm::Value *b = mul->getOperand(1);
                llvm::Value *c = add->getOperand(1);

                llvm::IRBuilder<> irb(add);
                llvm::Value *fma = irb.CreateIntrinsic(
                    llvm::Intrinsic::fmuladd, {a->getType()}, {a, b, c});

                add->replaceAllUsesWith(fma);
                add->eraseFromParent();
                if (mul->use_empty()) {
                  mul->eraseFromParent();
                }

                out = llvm::PreservedAnalyses::none();
              }
            }
          }
        }
      }
    }

    return out;
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMAPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "kurakin_fma") {
                    FPM.addPass(FMAPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
