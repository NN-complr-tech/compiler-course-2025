#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
// test

namespace {

struct FmulFaddMergePass final : llvm::PassInfoMixin<FmulFaddMergePass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &FAM) {
    bool codeChanged = false;

    struct ReplaceCandidate {
      llvm::BinaryOperator *faddInst;
      llvm::BinaryOperator *fmulInst;
      llvm::Value *otherOperand;
    };

    for (llvm::BasicBlock &block : F) {
      std::vector<ReplaceCandidate> candidates;

      auto isFMulOperand = [](llvm::Value *op,
                              llvm::Value *other) -> llvm::BinaryOperator * {
        if (auto *fmul = llvm::dyn_cast<llvm::BinaryOperator>(op)) {
          if (fmul->getOpcode() == llvm::Instruction::FMul) {
            return fmul;
          }
        }
        return nullptr;
      };

      // Collect potential replacements
      for (llvm::Instruction &inst : block) {
        if (auto *fadd = llvm::dyn_cast<llvm::BinaryOperator>(&inst)) {
          if (fadd->getOpcode() == llvm::Instruction::FAdd) {
            llvm::Value *op0 = fadd->getOperand(0);
            llvm::Value *op1 = fadd->getOperand(1);

            if (auto *fmul = isFMulOperand(op0, op1)) {
              if (fmul->hasAllowContract() && fadd->hasAllowContract()) {
                candidates.push_back({fadd, fmul, op1});
              }
            } else if (auto *fmul = isFMulOperand(op1, op0)) {
              if (fmul->hasAllowContract() && fadd->hasAllowContract()) {
                candidates.push_back({fadd, fmul, op0});
              }
            }
          }
        }
      }

      // Perform replacements
      for (const auto &candidate : candidates) {
        llvm::IRBuilder<> builder(candidate.faddInst);
        llvm::FastMathFlags flags = candidate.faddInst->getFastMathFlags();
        llvm::Value *fmuladd = builder.CreateIntrinsic(
            llvm::Intrinsic::fmuladd, candidate.fmulInst->getType(),
            {candidate.fmulInst->getOperand(0),
             candidate.fmulInst->getOperand(1), candidate.otherOperand});

        if (llvm::isa<llvm::FPMathOperator>(fmuladd)) {
          llvm::FPMathOperator *fmuladdInst =
              llvm::cast<llvm::FPMathOperator>(fmuladd);
        }

        candidate.faddInst->replaceAllUsesWith(fmuladd);
        candidate.faddInst->eraseFromParent();

        if (candidate.fmulInst->use_empty()) {
          candidate.fmulInst->eraseFromParent();
        }
        codeChanged = true;
      }
    }

    return codeChanged ? llvm::PreservedAnalyses::none()
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
                [](llvm::StringRef passName, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (passName == "FmulFaddMergePass") {
                    FPM.addPass(FmulFaddMergePass{});
                    return true;
                  }
                  return false;
                });
          }};
}