#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct ChangeADD : llvm::PassInfoMixin<ChangeADD> {
private:
  bool changed = false;

  void processBasicBlock(llvm::BasicBlock &BB, llvm::Function *addFunc) {
    for (auto it = BB.begin(); it != BB.end();) {
      llvm::Instruction *I = &*it++;
      if (llvm::BinaryOperator *BI = llvm::dyn_cast<llvm::BinaryOperator>(I)) {
        if (BI->getOpcode() == llvm::Instruction::Add) {
          replaceAddWithCall(BI, addFunc);
          it = BI->eraseFromParent();
          changed = true;
        }
      }
    }
  }

  void replaceAddWithCall(llvm::BinaryOperator *BI, llvm::Function *addFunc) {
    llvm::IRBuilder<> Builder(BI);
    std::string originalName = BI->hasName() ? BI->getName().str() : "";
    llvm::CallInst *Call =
        Builder.CreateCall(addFunc, {BI->getOperand(0), BI->getOperand(1)});
    if (!originalName.empty()) {
      Call->setName(originalName);
    }
    BI->replaceAllUsesWith(Call);
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    llvm::Module *M = F.getParent();
    llvm::Function *addFunc = M->getFunction("add");

    if (!addFunc || F.getName() == "add") {
      return llvm::PreservedAnalyses::all();
    }

    for (llvm::BasicBlock &BB : F) {
      processBasicBlock(BB, addFunc);
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ChangeADD", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "ChangeADD") {
                    FPM.addPass(ChangeADD{});
                    return true;
                  }
                  return false;
                });
          }};
}
