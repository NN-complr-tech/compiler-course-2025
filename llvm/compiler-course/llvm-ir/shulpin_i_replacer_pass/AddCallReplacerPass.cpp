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
    for (auto &F : M) {
      if (F.getName() == "add")
        continue;
      for (auto &BB : F) {
        for (auto It = BB.begin(), End = BB.end(); It != End;) {
          llvm::Instruction *I = &*It++;
          if (auto *BI = llvm::dyn_cast<llvm::BinaryOperator>(I)) {
            if (BI->getOpcode() == llvm::Instruction::Add) {
              if (addFunc && addFunc->arg_size() == 2) {
                llvm::Type *opTy = BI->getType();
                auto argIt = addFunc->arg_begin();
                llvm::Type *paramTy0 = argIt->getType();
                ++argIt;
                llvm::Type *paramTy1 = argIt->getType();
                if (paramTy0 == opTy && paramTy1 == opTy) {
                  llvm::IRBuilder<> Builder(BI);
                  llvm::Value *lhs = BI->getOperand(0);
                  llvm::Value *rhs = BI->getOperand(1);
                  llvm::CallInst *call =
                      Builder.CreateCall(addFunc, {lhs, rhs}, "sum");
                  BI->replaceAllUsesWith(call);
                  BI->eraseFromParent();
                  continue;
                }
              }
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
