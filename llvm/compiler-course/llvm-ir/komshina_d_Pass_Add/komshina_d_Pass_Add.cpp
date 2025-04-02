#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct Pass_Add : llvm::PassInfoMixin<Pass_Add> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    if (F.getName() == "add") {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Module *M = F.getParent();
    auto it = llvm::find_if(M->functions(), [](llvm::Function &Func) {
      return Func.getName() == "add" && Func.arg_size() == 2;
    });

    if (it == M->functions().end()) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Function *addFunction = &*it;

    bool modified = false;
    std::vector<llvm::Instruction *> toErase;

    for (llvm::BasicBlock &BB : F) {
      for (llvm::Instruction &I : BB) {
        if (auto *binOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          if (binOp->getOpcode() == llvm::Instruction::Add &&
              binOp->getOperand(0)->getType() ==
                  addFunction->getArg(0)->getType() &&
              binOp->getOperand(1)->getType() ==
                  addFunction->getArg(1)->getType()) {

            llvm::IRBuilder<> builder(binOp);
            llvm::Value *call = builder.CreateCall(
                addFunction, {binOp->getOperand(0), binOp->getOperand(1)});
            call->setName(binOp->getName());
            binOp->replaceAllUsesWith(call);
            toErase.push_back(binOp);
            modified = true;
          }
        }
      }
    }

    for (llvm::Instruction *I : toErase) {
      I->eraseFromParent();
    }

    return modified ? llvm::PreservedAnalyses::none()
                    : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Pass_Add", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "Pass_Add") {
                    FPM.addPass(Pass_Add{});
                    return true;
                  }
                  return false;
                });
          }};
}
