#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct AddReplacePass : PassInfoMixin<AddReplacePass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
    bool Changed = false;

    // Находим функцию add в модуле
    Function *TargetFunc = M.getFunction("add");
    if (!TargetFunc) {
      return PreservedAnalyses::all(); // Нет функции - никаких изменений
    }

    // Проверяем сигнатуру функции (должна быть i32)
    if (TargetFunc->arg_size() != 2 ||
        !TargetFunc->getReturnType()->isIntegerTy(32) ||
        !TargetFunc->getArg(0)->getType()->isIntegerTy(32) ||
        !TargetFunc->getArg(1)->getType()->isIntegerTy(32)) {
      return PreservedAnalyses::all();
    }

    // Получаем ссылку на функцию add
    FunctionCallee AddFunc =
        M.getOrInsertFunction("add", TargetFunc->getFunctionType());

    for (Function &F : M) {
      if (&F == TargetFunc)
        continue;

      for (BasicBlock &BB : F) {
        SmallVector<BinaryOperator *> AddInstructions;

        for (Instruction &I : BB) {
          if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
            if (BO->getOpcode() == Instruction::Add &&
                BO->getType()->isIntegerTy(32) &&
                BO->getOperand(0)->getType()->isIntegerTy(32) &&
                BO->getOperand(1)->getType()->isIntegerTy(32)) {
              AddInstructions.push_back(BO);
            }
          }
        }

        for (auto *AddInst : AddInstructions) {
          IRBuilder<> Builder(AddInst);
          Value *Args[] = {AddInst->getOperand(0), AddInst->getOperand(1)};
          CallInst *Call = Builder.CreateCall(AddFunc, Args);
          AddInst->replaceAllUsesWith(Call);
          AddInst->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AddReplacePass", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "add-replace") {
                    MPM.addPass(AddReplacePass());
                    return true;
                  }
                  return false;
                });
          }};
}
