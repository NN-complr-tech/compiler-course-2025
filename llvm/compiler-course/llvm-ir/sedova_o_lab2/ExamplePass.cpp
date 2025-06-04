#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct ReplaceAddWithCall : PassInfoMixin<ReplaceAddWithCall> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    StringRef FName = F.getName();

    if (FName == "add_i32" || FName == "add_i64" || FName == "add_float")
      return PreservedAnalyses::all();

    Module *M = F.getParent();
    if (!M)
      return PreservedAnalyses::all();

    std::string addFuncName;
    bool isFloat = false;

    Type *retType = F.getReturnType();
    if (retType->isIntegerTy(32))
      addFuncName = "add_i32";
    else if (retType->isIntegerTy(64))
      addFuncName = "add_i64";
    else if (retType->isFloatTy()) {
      addFuncName = "add_float";
      isFloat = true;
    } else {
      return PreservedAnalyses::all();
    }

    Function *addFunc = M->getFunction(addFuncName);
    if (!addFunc || addFunc->isDeclaration())
      return PreservedAnalyses::all();

    if (addFunc->arg_size() != 2)
      return PreservedAnalyses::all();

    auto argIter = addFunc->arg_begin();
    Type *argType1 = argIter->getType();
    ++argIter;
    Type *argType2 = argIter->getType();

    bool changed = false;

    for (auto &BB : F) {
      for (auto instIter = BB.begin(), end = BB.end(); instIter != end;) {
        Instruction *I = &*instIter++;
        if (auto *binOp = dyn_cast<BinaryOperator>(I)) {
          if ((!isFloat && binOp->getOpcode() == Instruction::Add) ||
              (isFloat && binOp->getOpcode() == Instruction::FAdd)) {
            Value *op1 = binOp->getOperand(0);
            Value *op2 = binOp->getOperand(1);

            if (op1->getType() == argType1 && op2->getType() == argType2) {
              IRBuilder<> builder(binOp);
              CallInst *call = builder.CreateCall(addFunc, {op1, op2});
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

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ReplaceAddWithCall", "0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) -> bool {
                  if (name == "replace-add") {
                    FPM.addPass(ReplaceAddWithCall{});
                    return true;
                  }
                  return false;
                });
          }};
}
