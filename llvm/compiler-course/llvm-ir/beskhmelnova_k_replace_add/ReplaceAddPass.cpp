#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct ReplaceAddPass : PassInfoMixin<ReplaceAddPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    if (F.getName() == "add") {
      return PreservedAnalyses::all();
    }

    bool changed = false;
    Module *M = F.getParent();

    Function *addFunction = nullptr;
    for (Function &Func : M->functions()) {
      if (Func.getName().contains("add") && Func.arg_size() == 2) {
        addFunction = &Func;
        break;
      }
    }

    if (!addFunction) {
      return PreservedAnalyses::all();
    }

    Type *expectedType = addFunction->getArg(0)->getType();

    for (BasicBlock &BB : F) {
      for (auto It = BB.begin(); It != BB.end();) {
        Instruction &I = *It++;
        if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
          if (binOp->getOpcode() == Instruction::Add) {

            if (binOp->getOperand(0)->getType() == expectedType &&
                binOp->getOperand(1)->getType() == expectedType) {

              std::string varName = binOp->getName().str();

              IRBuilder<> builder(binOp);
              Value *call = builder.CreateCall(
                  addFunction, {binOp->getOperand(0), binOp->getOperand(1)});

              call->setName(varName);

              binOp->replaceAllUsesWith(call);
              binOp->eraseFromParent();
              changed = true;
            }
          }
        }
      }
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ReplaceAddPass", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (name == "ReplaceAddPass") {
                    FPM.addPass(ReplaceAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
