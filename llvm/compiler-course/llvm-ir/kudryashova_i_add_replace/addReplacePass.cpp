#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct AddReplacePass : llvm::PassInfoMixin<AddReplacePass> {
  llvm::PreservedAnalyses run(llvm::Module &Mod,
                              llvm::ModuleAnalysisManager &) {
    bool Changed = false;

    llvm::Function *AddFunc = Mod.getFunction("add");
    if (!AddFunc || AddFunc->arg_size() != 2) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Type *Arg0Type = AddFunc->getArg(0)->getType();
    llvm::Type *Arg1Type = AddFunc->getArg(1)->getType();

    for (llvm::Function &F : Mod) {
      if (&F == AddFunc) {
        continue;
      }
      for (llvm::BasicBlock &BB : F) {
        llvm::SmallVector<llvm::BinaryOperator *> ToReplace;

        for (llvm::Instruction &I : BB) {
          if (auto *BinOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
            if (BinOp->getOpcode() == llvm::Instruction::Add) {
              llvm::Value *Op0 = BinOp->getOperand(0);
              llvm::Value *Op1 = BinOp->getOperand(1);
              if (Op0->getType() == Arg0Type && Op1->getType() == Arg1Type) {
                ToReplace.push_back(BinOp);
              }
            }
          }
        }

        for (auto *BinOp : ToReplace) {
          llvm::IRBuilder<> Builder(BinOp);
          llvm::Value *Args[] = {BinOp->getOperand(0), BinOp->getOperand(1)};
          llvm::CallInst *Call = Builder.CreateCall(AddFunc, Args);
          BinOp->replaceAllUsesWith(Call);
          BinOp->eraseFromParent();
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
  return {LLVM_PLUGIN_API_VERSION, "AddReplacePass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::ModulePassManager &MPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "AddReplacePass") {
                    MPM.addPass(AddReplacePass{});
                    return true;
                  }
                  return false;
                });
          }};
}
