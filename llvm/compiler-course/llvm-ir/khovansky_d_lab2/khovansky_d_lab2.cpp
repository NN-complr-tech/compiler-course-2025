
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace {

struct ReplaceAddInstructionPass
    : llvm::PassInfoMixin<ReplaceAddInstructionPass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    llvm::Module *Mod = Func.getParent();

    if (!Mod) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Function *AddFunc = Mod->getFunction("add");

    if (!AddFunc || AddFunc == &Func || AddFunc->getName() != "add") {
      return llvm::PreservedAnalyses::all();
    }

    // проверка на сигнатуру: не vararg и два аргумента
    if (AddFunc->arg_size() != 2 || AddFunc->isVarArg()) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::SmallVector<llvm::Instruction *, 8> ToReplace;

    llvm::FunctionType *AddFuncType = AddFunc->getFunctionType();

    for (llvm::BasicBlock &Block : Func) {
      for (llvm::Instruction &Inst : Block) {
        llvm::BinaryOperator *BinOp =
            llvm::dyn_cast<llvm::BinaryOperator>(&Inst);

        if (!BinOp || BinOp->getOpcode() != llvm::Instruction::Add) {
          continue;
        }

        // вызов функции в этом случае менее эффективен, чем встроенная операция
        if (llvm::isa<llvm::Constant>(BinOp->getOperand(0)) ||
            llvm::isa<llvm::Constant>(BinOp->getOperand(1))) {
          continue;
        }

        llvm::Type *LeftType = BinOp->getOperand(0)->getType();
        llvm::Type *RightType = BinOp->getOperand(1)->getType();
        llvm::Type *ResultType = BinOp->getType();

        if (AddFuncType->getParamType(0) == LeftType &&
            AddFuncType->getParamType(1) == RightType &&
            AddFuncType->getReturnType() == ResultType) {
          ToReplace.push_back(BinOp);
        }
      }
    }

    if (ToReplace.empty()) {
      return llvm::PreservedAnalyses::all();
    }

    for (llvm::Instruction *OldAdd : ToReplace) {
      llvm::IRBuilder<> Builder(OldAdd);
      llvm::Value *Arg1 = OldAdd->getOperand(0);
      llvm::Value *Arg2 = OldAdd->getOperand(1);

      llvm::CallInst *Call =
          Builder.CreateCall(AddFunc, {Arg1, Arg2}, OldAdd->getName());

      OldAdd->replaceAllUsesWith(Call);
      OldAdd->eraseFromParent();
    }

    return llvm::PreservedAnalyses::none();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ReplaceAddInstructionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "AddReplacer") {
                    FPM.addPass(ReplaceAddInstructionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
