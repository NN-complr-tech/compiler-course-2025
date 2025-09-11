#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace {
struct ReplaceAddPass : llvm::PassInfoMixin<ReplaceAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    if (func.getName() == "add") {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Module *mod = func.getParent();
    if (!mod)
      return llvm::PreservedAnalyses::all();

    llvm::Function *addFunc = mod->getFunction("add");
    if (!addFunc) {
      return llvm::PreservedAnalyses::all();
    }

    if (addFunc->arg_size() != 2 && addFunc->isVarArg()) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Type *arg1Type = addFunc->getArg(0)->getType();
    llvm::Type *arg2Type = addFunc->getArg(1)->getType();
    llvm::Type *returnType = addFunc->getReturnType();

    llvm::SmallVector<llvm::Instruction *, 8> instructionsToReplace;
    bool changed = false;

    for (llvm::BasicBlock &BB : func) {
      for (llvm::Instruction &I : BB) {
        if (auto *BO = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {

          if (llvm::isa<llvm::Constant>(BO->getOperand(0)) ||
            llvm::isa<llvm::Constant>(BO->getOperand(1))) {
            continue;
          }

          if (BO->getOpcode() == llvm::Instruction::Add) {
            if (BO->getType() == returnType &&
                BO->getOperand(0)->getType() == arg1Type &&
                BO->getOperand(1)->getType() == arg2Type) {
              instructionsToReplace.push_back(BO);
            }
          }
        }
      }
    }

    for (llvm::Instruction *addInst : instructionsToReplace) {
      llvm::IRBuilder<> builder(addInst);
      llvm::Value *call = builder.CreateCall(
      addFunc,
      {addInst->getOperand(0), addInst->getOperand(1)}
      );
      call->setName(addInst->getName());

      addInst->replaceAllUsesWith(call);
      addInst->eraseFromParent();
      changed = true;
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ReplaceAddPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "addReplacePass") {
                    FPM.addPass(ReplaceAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
