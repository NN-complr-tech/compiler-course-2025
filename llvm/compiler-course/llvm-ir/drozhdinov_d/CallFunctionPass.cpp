#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace {
struct CallFunctionPass : llvm::PassInfoMixin<CallFunctionPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool status = false;
    llvm::Module *mod = func.getParent();
    llvm::Function *CallFunc = mod->getFunction("add");
    llvm::SmallVector<llvm::BinaryOperator*, 16> WorkList;
    if (!CallFunc || &func == CallFunc) {
      return llvm::PreservedAnalyses::all();
    }

    for (auto &block : func) {
      for (auto &inst : block) {
        if (auto *op = llvm::dyn_cast<llvm::BinaryOperator>(&inst)) {
          if (op->getOpcode() == llvm::Instruction::Add) {
            if (CallFunc->getFunctionType()->getNumParams() == 2 &&
                CallFunc->getFunctionType()->getParamType(0) ==
                    op->getOperand(0)->getType() &&
                CallFunc->getFunctionType()->getParamType(1) ==
                    op->getOperand(1)->getType() &&
                CallFunc->getFunctionType()->getReturnType() == op->getType()) {
              WorkList.push_back(op);
            }
          }
        }
      }
    }

    for (auto *op : WorkList) {
      llvm::IRBuilder<> Builder(op);
      llvm::Value *call = Builder.CreateCall(
          CallFunc, {op->getOperand(0), op->getOperand(1)}, op->getName());
      op->replaceAllUsesWith(call);
      op->eraseFromParent();
      status = true;
    }

    return status ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CallFunctionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "callfunc") {
                    FPM.addPass(CallFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
