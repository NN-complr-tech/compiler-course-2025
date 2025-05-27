#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct AddCallReplacerPass : llvm::PassInfoMixin<AddCallReplacerPass> {
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
    llvm::Function *addFunc = M.getFunction("add");
    bool canReplace = false;
    llvm::Type *paramTy = nullptr;
    if (addFunc && addFunc->arg_size() == 2) {
      auto AI = addFunc->arg_begin();
      llvm::Type *t0 = AI->getType();
      ++AI;
      llvm::Type *t1 = AI->getType();
      if (t0 == t1) {
        canReplace = true;
        paramTy = t0;
      }
    }

    for (llvm::Function &F : M) {
      if (&F == addFunc)
        continue;
      for (llvm::BasicBlock &BB : F) {
        for (llvm::Instruction &I : llvm::make_early_inc_range(BB)) {
          if (!canReplace)
            break;
          if (auto *BI = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
            if (BI->getOpcode() == llvm::Instruction::Add &&
                BI->getType() == paramTy) {
              llvm::IRBuilder<> Builder(&I);
              llvm::Value *L = BI->getOperand(0);
              llvm::Value *R = BI->getOperand(1);
              llvm::CallInst *CI = Builder.CreateCall(addFunc, {L, R}, "sum");
              BI->replaceAllUsesWith(CI);
              BI->eraseFromParent();
            }
          }
        }
      }
    }

    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AddCallReplacer", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::ModulePassManager &MPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (name == "ReplacerPass") {
                    MPM.addPass(AddCallReplacerPass());
                    return true;
                  }
                  return false;
                });
          }};
}
