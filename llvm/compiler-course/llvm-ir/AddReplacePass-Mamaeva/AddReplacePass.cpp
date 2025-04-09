#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct AddReplacePass : llvm::PassInfoMixin<AddReplacePass> {
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    bool Changed = false;

    // Находим функцию add в модуле
    llvm::Function *TargetFunc = M.getFunction("add");
    if (!TargetFunc) {
      return llvm::PreservedAnalyses::all(); // Нет функции - никаких изменений
    }

    // Проверяем сигнатуру функции
    if (TargetFunc->arg_size() != 2 ||
        !TargetFunc->getReturnType()->isIntegerTy(32) ||
        !TargetFunc->getArg(0)->getType()->isIntegerTy(32) ||
        !TargetFunc->getArg(1)->getType()->isIntegerTy(32)) {
      return llvm::PreservedAnalyses::all();
    }

    for (llvm::Function &F : M) {
      if (&F == TargetFunc)
        continue;

      for (llvm::BasicBlock &BB : F) {
        llvm::SmallVector<llvm::BinaryOperator *> AddInstructions;

        for (llvm::Instruction &I : BB) {
          if (auto *BO = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
            if (BO->getOpcode() == llvm::Instruction::Add &&
                BO->getType()->isIntegerTy(32) &&
                BO->getOperand(0)->getType()->isIntegerTy(32) &&
                BO->getOperand(1)->getType()->isIntegerTy(32)) {
              AddInstructions.push_back(BO);
            }
          }
        }

        for (auto *AddInst : AddInstructions) {
          llvm::IRBuilder<> Builder(AddInst);
          llvm::Value *Args[] = {AddInst->getOperand(0),
                                 AddInst->getOperand(1)};
          llvm::CallInst *Call = Builder.CreateCall(TargetFunc, Args);
          AddInst->replaceAllUsesWith(Call);
          AddInst->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace
