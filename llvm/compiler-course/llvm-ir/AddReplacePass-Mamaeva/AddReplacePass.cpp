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
  PreservedAnalyses run(Module &Mod, ModuleAnalysisManager &) {
    bool Changed = false;

    // Находим функцию add в модуле
    Function *AddFunc = Mod.getFunction("add");
    if (!AddFunc) {
      return PreservedAnalyses::all(); // Нет функции - никаких изменений
    }

    // Проверяем сигнатуру функции (должна быть i32(i32, i32))
    if (AddFunc->arg_size() != 2 ||
        !AddFunc->getReturnType()->isIntegerTy(32) ||
        !AddFunc->getArg(0)->getType()->isIntegerTy(32) ||
        !AddFunc->getArg(1)->getType()->isIntegerTy(32)) {
      return PreservedAnalyses::all();
    }

    // Получаем ссылку на функцию add с правильным типом
    FunctionCallee AddFuncCallee =
        Mod.getOrInsertFunction("add", AddFunc->getFunctionType());

    for (Function &F : Mod) {
      if (&F == AddFunc)
        continue;

      for (BasicBlock &BB : F) {
        SmallVector<BinaryOperator *> ToReplace;

        for (Instruction &I : BB) {
          if (auto *BinOp = dyn_cast<BinaryOperator>(&I)) {
            if (BinOp->getOpcode() == Instruction::Add &&
                BinOp->getType()->isIntegerTy(32) &&
                BinOp->getOperand(0)->getType()->isIntegerTy(32) &&
                BinOp->getOperand(1)->getType()->isIntegerTy(32)) {
              ToReplace.push_back(BinOp);
            }
          }
        }

        for (auto *BinOp : ToReplace) {
          IRBuilder<> Builder(BinOp);
          Value *Args[] = {BinOp->getOperand(0), BinOp->getOperand(1)};
          CallInst *Call = Builder.CreateCall(AddFuncCallee, Args);
          Call->setDebugLoc(BinOp->getDebugLoc());
          BinOp->replaceAllUsesWith(Call);
          BinOp->eraseFromParent();
          Changed = true;
        }
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION, "AddReplacePass", "v0.1", [](PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, ModulePassManager &MPM,
               ArrayRef<PassBuilder::PipelineElement>) {
              if (Name == "add-replace") { // Изменено для соответствия тесту
                MPM.addPass(AddReplacePass());
                return true;
              }
              return false;
            });
      }};
}
