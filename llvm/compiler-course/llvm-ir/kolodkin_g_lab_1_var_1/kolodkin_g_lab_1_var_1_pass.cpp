#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace {

class FusedMultiplyAddPass : public llvm::PassInfoMixin<FusedMultiplyAddPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &) {
    bool modified = false;

    for (auto &BB : F) {
      for (auto I = BB.begin(); I != BB.end(); ) {
        llvm::Instruction &Inst = *I++;
        if (auto *FAdd = llvm::dyn_cast<llvm::BinaryOperator>(&Inst)) {
          if (FAdd->getOpcode() == llvm::Instruction::FAdd) {
            for (int idx = 0; idx < 2; ++idx) {
              if (auto *FMul = llvm::dyn_cast<llvm::BinaryOperator>(FAdd->getOperand(idx))) {
                if (FMul->getOpcode() == llvm::Instruction::FMul) {
                  llvm::IRBuilder<> Builder(FAdd);
                  auto *fmaInst = Builder.CreateIntrinsic(
                    llvm::Intrinsic::fmuladd,
                    {FMul->getType()},
                    {FMul->getOperand(0), FMul->getOperand(1), FAdd->getOperand(1 - idx)});
                  FAdd->replaceAllUsesWith(fmaInst);
                  FAdd->eraseFromParent();
                  if (FMul->use_empty()) {
                    FMul->eraseFromParent();
                  }
                  modified = true;
                  break;
                }
              }
            }
          }
        }
      }
    }

    return modified ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FusedMultuplyAddPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "add-and-mul-fma") {
                    FPM.addPass(FusedMultiplyAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
