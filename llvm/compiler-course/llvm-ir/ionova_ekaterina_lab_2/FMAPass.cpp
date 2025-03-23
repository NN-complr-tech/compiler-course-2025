#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FMAPass : llvm::PassInfoMixin<FMAPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;

    for (llvm::BasicBlock &BB : F) {
      llvm::SmallVector<llvm::BinaryOperator *, 8> Candidates;

      for (llvm::Instruction &I : BB) {
        if (auto *AddOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          if (AddOp->getOpcode() != llvm::Instruction::FAdd) {
            continue;
          }

          for (unsigned i = 0; i < 2; ++i) {
            if (auto *MultiplyOp = llvm::dyn_cast<llvm::BinaryOperator>(
                    AddOp->getOperand(i))) {
              if (MultiplyOp->getOpcode() == llvm::Instruction::FMul &&
                  canReplaceFMul(MultiplyOp, AddOp)) {
                Candidates.push_back(AddOp);
              }
            }
          }
        }
      }

      for (auto *AddOp : Candidates) {
        llvm::BinaryOperator *MultiplyOp = nullptr;
        llvm::Value *Addend = nullptr;

        for (unsigned i = 0; i < 2; ++i) {
          if (auto *Op =
                  llvm::dyn_cast<llvm::BinaryOperator>(AddOp->getOperand(i))) {
            if (Op->getOpcode() == llvm::Instruction::FMul) {
              MultiplyOp = Op;
              Addend = AddOp->getOperand(1 - i);
              break;
            }
          }
        }

        if (!MultiplyOp || !Addend) {
          continue;
        }

        replaceWithFMA(AddOp, MultiplyOp, Addend);
        Changed = true;
      }
    }

    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }

private:
  bool canReplaceFMul(llvm::BinaryOperator *MultiplyOp,
                      llvm::BinaryOperator *AddOp) {
    return MultiplyOp->getOpcode() == llvm::Instruction::FMul &&
           std::distance(MultiplyOp->user_begin(), MultiplyOp->user_end()) == 1;
  }

  void replaceWithFMA(llvm::BinaryOperator *AddOp,
                      llvm::BinaryOperator *MultiplyOp, llvm::Value *Addend) {
    llvm::IRBuilder<> Builder(AddOp);
    llvm::Value *FMA = Builder.CreateIntrinsic(
        llvm::Intrinsic::fmuladd, {MultiplyOp->getType()},
        {MultiplyOp->getOperand(0), MultiplyOp->getOperand(1), Addend}, nullptr,
        "fma");

    AddOp->replaceAllUsesWith(FMA);
    AddOp->eraseFromParent();

    if (MultiplyOp->use_empty()) {
      MultiplyOp->eraseFromParent();
    }
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMAPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "fmapass") {
                    FPM.addPass(FMAPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
