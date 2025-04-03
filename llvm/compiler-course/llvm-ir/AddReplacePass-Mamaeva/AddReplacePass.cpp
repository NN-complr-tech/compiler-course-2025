#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct AddReplacePass : llvm::PassInfoMixin<AddReplacePass> {
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &) {
    bool Changed = false;

    // Находим функцию add в модуле
    llvm::Function *TargetFunc = M.getFunction("add");
    if (!TargetFunc || TargetFunc->arg_size() != 2) {
      return llvm::PreservedAnalyses::all();
    }

    // Получаем типы аргументов целевой функции
    llvm::Type *ArgTy1 = TargetFunc->getArg(0)->getType();
    llvm::Type *ArgTy2 = TargetFunc->getArg(1)->getType();

    for (llvm::Function &F : M) {
      // Пропускаем саму функцию add
      if (&F == TargetFunc) continue;

      for (llvm::BasicBlock &BB : F) {
        std::vector<llvm::BinaryOperator*> AddInstructions;

        // Собираем все инструкции add для замены
        for (llvm::Instruction &I : BB) {
          if (auto *BO = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
            if (BO->getOpcode() == llvm::Instruction::Add) {
              // Проверяем соответствие типов
              if (BO->getOperand(0)->getType() == ArgTy1 &&
                  BO->getOperand(1)->getType() == ArgTy2) {
                AddInstructions.push_back(BO);
              }
            }
          }
        }

        // Выполняем замену
        for (auto *AddInst : AddInstructions) {
          llvm::IRBuilder<> Builder(AddInst);
          llvm::Value *Args[] = {AddInst->getOperand(0), AddInst->getOperand(1)};
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

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION,
      "AddReplacePass",  
      "0.1",
      [](llvm::PassBuilder &PB) {
        PB.registerPipelineParsingCallback(
            [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
               llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
              if (Name == "add-replace") {  // Имя для вызова пасса
                MPM.addPass(AddReplacePass{});
                return true;
              }
              return false;
            });
      }};
}
