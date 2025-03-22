#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FmulFaddMergePass : llvm::PassInfoMixin<FmulFaddMergePass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool changed = false;
    for (llvm::BasicBlock &bb : func) {
      std::vector<llvm::BinaryOperator *> toReplace;
      for (llvm::Instruction &inst : bb) {
        if (auto *fadd = llvm::dyn_cast<llvm::BinaryOperator>(&inst)) {
          if (fadd->getOpcode() == llvm::Instruction::FAdd) {
            if (auto *fmul =
                    llvm::dyn_cast<llvm::BinaryOperator>(fadd->getOperand(0))) {
              if (fmul->getOpcode() == llvm::Instruction::FMul) {
                toReplace.push_back(fadd);
              }
            } else if (auto *fmul = llvm::dyn_cast<llvm::BinaryOperator>(
                           fadd->getOperand(1))) {
              if (fmul->getOpcode() == llvm::Instruction::FMul) {
                toReplace.push_back(fadd);
              }
            }
          }
        }
      }

      for (auto *fadd : toReplace) {
        llvm::BinaryOperator *fmul = nullptr;
        llvm::Value *thirdOp = nullptr;

        if (auto *op =
                llvm::dyn_cast<llvm::BinaryOperator>(fadd->getOperand(0))) {
          if (op->getOpcode() == llvm::Instruction::FMul) {
            fmul = op;
            thirdOp = fadd->getOperand(1);
          }
        } else if (auto *op = llvm::dyn_cast<llvm::BinaryOperator>(
                       fadd->getOperand(1))) {
          if (op->getOpcode() == llvm::Instruction::FMul) {
            fmul = op;
            thirdOp = fadd->getOperand(0);
          }
        }

        if (!fmul || !thirdOp) {
          continue;
        }

        llvm::IRBuilder<> leBuilder(fadd);
        llvm::Value *fmulfaddmerged = leBuilder.CreateIntrinsic(
            llvm::Intrinsic::fmuladd, fmul->getType(),
            {fmul->getOperand(0), fmul->getOperand(1), thirdOp});
        fadd->replaceAllUsesWith(fmulfaddmerged);
        fadd->eraseFromParent();
        if (fmul->use_empty()) {
          fmul->eraseFromParent();
        }

        changed = true;
      }
    }
    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FmulFaddMergePass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "FmulFaddMergePass") {
                    FPM.addPass(FmulFaddMergePass{});
                    return true;
                  }
                  return false;
                });
          }};
}
