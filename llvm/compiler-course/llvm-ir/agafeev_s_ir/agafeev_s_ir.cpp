#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

llvm::BinaryOperator *isFmulInstruction(llvm::Value *Operand) {
  if (auto *OpInst = llvm::dyn_cast<llvm::BinaryOperator>(Operand)) {
    if (OpInst->getOpcode() == llvm::Instruction::FMul)
      return OpInst;
  }
  return nullptr;
}

struct FmaPass : llvm::PassInfoMixin<FmaPass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;

    struct FmaReplace {
      llvm::BinaryOperator *FAdd;
      llvm::BinaryOperator *FMul;
      llvm::Value *Other;
    };

    for (llvm::BasicBlock &BB : Func) {
      std::vector<FmaReplace> ReplaceVector;

      auto CheckOperand = [](llvm::Value *Operand, llvm::Value *OtherOperand)
          -> llvm::BinaryOperator * { isFmulInstruction(Operand); };

      for (llvm::Instruction &Inst : BB) {
        if (auto *FAdd = llvm::dyn_cast<llvm::BinaryOperator>(&Inst)) {
          if (FAdd->getOpcode() == llvm::Instruction::FAdd) {
            llvm::Value *Op0 = FAdd->getOperand(0);
            llvm::Value *Op1 = FAdd->getOperand(1);

            if (auto *FMul = CheckOperand(Op0, Op1)) {
              ReplaceVector.push_back({FAdd, FMul, Op1});
            } else if (auto *FMul = CheckOperand(Op1, Op0)) {
              ReplaceVector.push_back({FAdd, FMul, Op0});
            }
          }
        }
      }

      for (const auto &Info : ReplaceVector) {
        llvm::IRBuilder<> Builder(Info.FAdd);
        llvm::Value *FMulAdd = Builder.CreateIntrinsic(
            llvm::Intrinsic::fmuladd, Info.FMul->getType(),
            {Info.FMul->getOperand(0), Info.FMul->getOperand(1), Info.Other});

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
  return {LLVM_PLUGIN_API_VERSION, "FmaPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "FmaPass") {
                    FPM.addPass(FmaPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
