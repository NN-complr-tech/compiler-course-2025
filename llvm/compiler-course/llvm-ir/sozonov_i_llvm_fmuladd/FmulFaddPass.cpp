#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FmulFaddPass : llvm::PassInfoMixin<FmulFaddPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool isModified = false;
    llvm::SmallVector<llvm::Instruction *, 4> instructionsToRemove;

    for (llvm::BasicBlock &BB : func) {
      for (llvm::Instruction &I : llvm::make_early_inc_range(BB)) {
        if (I.getOpcode() != llvm::Instruction::FAdd)
          continue;

        llvm::Value *Op1 = I.getOperand(0);
        llvm::Value *Op2 = I.getOperand(1);

        llvm::Instruction *MulInst = nullptr;
        llvm::Value *A = nullptr, *B = nullptr, *C = nullptr;

        llvm::Value *Operands[2] = {Op1, Op2};

        for (int i = 0; i < 2; ++i) {
          if (auto *Mul = llvm::dyn_cast<llvm::Instruction>(Operands[i])) {
            if (Mul->getOpcode() == llvm::Instruction::FMul) {
              A = Mul->getOperand(0);
              B = Mul->getOperand(1);
              C = Operands[1 - i];
              MulInst = Mul;
              break;
            }
          }
        }

        if (MulInst) {
          if (!MulInst->hasOneUse())
            continue;

          llvm::IRBuilder<> builder(&I);
          llvm::Function *FmaFunc = llvm::Intrinsic::getDeclaration(
              func.getParent(), llvm::Intrinsic::fmuladd, I.getType());

          llvm::Value *Fma = builder.CreateCall(FmaFunc, {A, B, C});
          I.replaceAllUsesWith(Fma);
          instructionsToRemove.push_back(&I);
          instructionsToRemove.push_back(MulInst);

          isModified = true;
        }
      }
    }

    for (llvm::Instruction *I : instructionsToRemove)
      I->eraseFromParent();

    return isModified ? llvm::PreservedAnalyses::none()
                      : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FmulFaddPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "FmulFaddPass") {
                    FPM.addPass(FmulFaddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
