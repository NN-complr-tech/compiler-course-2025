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

    llvm::Module *Mod = F.getParent();
    auto it = llvm::find_if(Mod->functions(), [](llvm::Function &Func) {
      return Func.getName() == "add" && Func.arg_size() == 2;
    });

    if (it == Mod->functions().end()) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::Function *addFunction = &*it;

    bool modified = false;
    std::vector<llvm::Instruction *> toErase;

    for (auto& block : F) {
      for (auto& instr : block) {
        if (auto* binaryOp = llvm::dyn_cast<llvm::BinaryOperator>(&instr)) {
          if (binaryOp->getOpcode() == llvm::Instruction::Add) {
            auto* firstOperand = binaryOp->getOperand(0);
            auto* secondOperand = binaryOp->getOperand(1);

            if (firstOperand->getType() == addFunction->getArg(0)->getType() &&
              secondOperand->getType() == addFunction->getArg(1)->getType()) {

              llvm::IRBuilder<> builder(binaryOp);
              llvm::Value* functionCall = builder.CreateCall(
                  addFunction, { firstOperand, secondOperand });
              functionCall->setName(binaryOp->getName());

              binaryOp->replaceAllUsesWith(functionCall);
              toErase.push_back(binaryOp);
              modified = true;
            }
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
