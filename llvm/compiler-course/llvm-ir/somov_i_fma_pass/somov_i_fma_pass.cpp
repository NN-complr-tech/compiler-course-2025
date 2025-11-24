#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct FMARewriterPass : llvm::PassInfoMixin<FMARewriterPass> {
private:
  bool tryToFuseFMA(
      llvm::Instruction &addInstruction,
      llvm::SmallVector<llvm::Instruction *, 4> &obsoleteInstructions) {
    if (addInstruction.getOpcode() != llvm::Instruction::FAdd) {
      return false;
    }

    llvm::Value *leftOperand = addInstruction.getOperand(0);
    llvm::Value *rightOperand = addInstruction.getOperand(1);

    auto *mulInstruction = llvm::dyn_cast<llvm::Instruction>(leftOperand);
    llvm::Value *addend = rightOperand;

    if (!mulInstruction ||
        mulInstruction->getOpcode() != llvm::Instruction::FMul) {
      mulInstruction = llvm::dyn_cast<llvm::Instruction>(rightOperand);
      addend = leftOperand;
    }

    if (!mulInstruction ||
        mulInstruction->getOpcode() != llvm::Instruction::FMul) {
      return false;
    }

    if (!mulInstruction->hasOneUse()) {
      return false;
    }

    llvm::Value *multiplicand1 = mulInstruction->getOperand(0);
    llvm::Value *multiplicand2 = mulInstruction->getOperand(1);

    llvm::IRBuilder<> builder(&addInstruction);

    llvm::Function *fmaIntrinsic = llvm::Intrinsic::getDeclaration(
        addInstruction.getModule(), llvm::Intrinsic::fmuladd,
        addInstruction.getType());

    llvm::Value *fmaCall = builder.CreateCall(
        fmaIntrinsic, {multiplicand1, multiplicand2, addend});

    addInstruction.replaceAllUsesWith(fmaCall);

    obsoleteInstructions.push_back(&addInstruction);
    obsoleteInstructions.push_back(mulInstruction);

    return true;
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool irModified = false;
    llvm::SmallVector<llvm::Instruction *, 4> obsoleteInstructions;

    for (llvm::BasicBlock &BB : F) {
      for (llvm::Instruction &I : llvm::make_early_inc_range(BB)) {
        if (tryToFuseFMA(I, obsoleteInstructions)) {
          irModified = true;
        }
      }
    }

    for (llvm::Instruction *I : obsoleteInstructions) {
      I->eraseFromParent();
    }

    return irModified ? llvm::PreservedAnalyses::none()
                      : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMARewriterPass", "1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "FMARewriterPass") {
                    FPM.addPass(FMARewriterPass{});
                    return true;
                  }
                  return false;
                });
          }};
}