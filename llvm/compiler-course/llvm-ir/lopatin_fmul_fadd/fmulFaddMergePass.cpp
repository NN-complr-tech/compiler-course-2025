#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FmulFaddMergePass : llvm::PassInfoMixin<FmulFaddMergePass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;

    struct ReplacementStruct {
      llvm::BinaryOperator *FAdd;
      llvm::BinaryOperator *FMul;
      llvm::Value *ThirdOperand;
    };

    for (llvm::BasicBlock &BB : Func) {
      std::vector<ReplacementStruct> ToReplace;

      auto CheckOperand =
          [](llvm::Value *Operand,
             llvm::Value *OtherOperand) -> llvm::BinaryOperator * {
        if (auto *FMul = llvm::dyn_cast<llvm::BinaryOperator>(Operand)) {
          if (FMul->getOpcode() == llvm::Instruction::FMul) {
            return FMul;
          }
        }
        return nullptr;
      };

      // 1st pass: Collect replacements
      for (llvm::Instruction &Inst : BB) {
        if (auto *FAdd = llvm::dyn_cast<llvm::BinaryOperator>(&Inst)) {
          if (FAdd->getOpcode() == llvm::Instruction::FAdd) {
            llvm::Value *Op0 = FAdd->getOperand(0);
            llvm::Value *Op1 = FAdd->getOperand(1);

            if (auto *FMul = CheckOperand(Op0, Op1)) {
              ToReplace.push_back({FAdd, FMul, Op1});
            } else if (auto *FMul = CheckOperand(Op1, Op0)) {
              ToReplace.push_back({FAdd, FMul, Op0});
            }
          }
        }
      }

      // 2nd pass, replacing
      for (const auto &Info : ToReplace) {
        llvm::IRBuilder<> Builder(Info.FAdd);
        llvm::Value *FMulAdd = Builder.CreateIntrinsic(
            llvm::Intrinsic::fmuladd, Info.FMul->getType(),
            {Info.FMul->getOperand(0), Info.FMul->getOperand(1),
             Info.ThirdOperand});

        Info.FAdd->replaceAllUsesWith(FMulAdd);
        Info.FAdd->eraseFromParent();

        if (Info.FMul->use_empty()) {
          Info.FMul->eraseFromParent();
        }

        Changed = true;
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
  return {LLVM_PLUGIN_API_VERSION, "FmulFaddMergePass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "FmulFaddMergePass") {
                    FPM.addPass(FmulFaddMergePass{});
                    return true;
                  }
                  return false;
                });
          }};
}
