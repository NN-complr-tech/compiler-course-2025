#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct LlvmFmaPass : llvm::PassInfoMixin<LlvmFmaPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &AM) {
    bool Changed = false;
    for (llvm::BasicBlock &BB : func) {
      for (llvm::Instruction &Inst : llvm::make_early_inc_range(BB)) {
        auto *AddOp = llvm::dyn_cast<llvm::BinaryOperator>(&Inst);
        if (!AddOp || AddOp->getOpcode() != llvm::Instruction::FAdd) {
          continue;
        }
        for (unsigned i = 0; i < 2; ++i) {
          llvm::Value *Operand = AddOp->getOperand(i);
          auto *MulOp = llvm::dyn_cast<llvm::BinaryOperator>(Operand);

          if (MulOp && MulOp->getOpcode() == llvm::Instruction::FMul) {
            llvm::Value *OpA = MulOp->getOperand(0);
            llvm::Value *OpB = MulOp->getOperand(1);
            llvm::Value *OpC = AddOp->getOperand(1 - i);

            if (MulOp->getType() != OpA->getType() ||
                MulOp->getType() != OpB->getType() ||
                MulOp->getType() != OpC->getType()) {
              continue;
            }
            llvm::IRBuilder<> Builder(AddOp);
            llvm::Value *FmaResult = Builder.CreateIntrinsic(
                llvm::Intrinsic::fmuladd, {MulOp->getType()}, {OpA, OpB, OpC});
            FmaResult->takeName(AddOp);
            AddOp->replaceAllUsesWith(FmaResult);
            AddOp->eraseFromParent();
            if (MulOp->use_empty()) {
              MulOp->eraseFromParent();
            }
            Changed = true;
            break;
          }
        }
      }
    }
    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LlvmFmaPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "LlvmFmaPass") {
                    FPM.addPass(LlvmFmaPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
