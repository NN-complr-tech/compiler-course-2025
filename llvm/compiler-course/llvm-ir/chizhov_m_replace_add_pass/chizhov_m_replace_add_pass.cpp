#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {
struct ReplaceAddPass : llvm::PassInfoMixin<ReplaceAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    llvm::Module *module = func.getParent();
    llvm::Function *addFunc = module->getFunction("add");

    if (!addFunc || &func == addFunc) {
      return llvm::PreservedAnalyses::all();
    }

    bool change = false;
    llvm::SmallVector<llvm::BinaryOperator *, 16> operators;

    for (llvm::BasicBlock &block : func) {
      for (llvm::Instruction &instr : block) {
        if (auto *binOp = llvm::dyn_cast<llvm::BinaryOperator>(&instr)) {
          if (binOp->getOpcode() == llvm::Instruction::Add) {
            operators.push_back(binOp);
          }
        }
      }
    }

    for (auto *binOp : operators) {
      if (addFunc->getFunctionType()->getNumParams() == 2 &&
          addFunc->getFunctionType()->getParamType(0) ==
              binOp->getOperand(0)->getType() &&
          addFunc->getFunctionType()->getParamType(1) ==
              binOp->getOperand(1)->getType() &&
          addFunc->getFunctionType()->getReturnType() == binOp->getType()) {

        llvm::IRBuilder<> builder(binOp);
        llvm::Value *call = builder.CreateCall(
            addFunc, {binOp->getOperand(0), binOp->getOperand(1)},
            binOp->getName());

        binOp->replaceAllUsesWith(call);
        binOp->eraseFromParent();
        change = true;
      }
    }

    return change ? llvm::PreservedAnalyses::none()
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
                  if (name == "ReplaceAddPass") {
                    FPM.addPass(ReplaceAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
