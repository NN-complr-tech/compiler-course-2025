#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

void visitor(llvm::Function &F) {
  llvm::SmallVector<llvm::BinaryOperator *, 8> MulSubsToReplace;

  for (auto &BB : F) {
    for (auto &I : BB) {
      if (auto *SubOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
        if (SubOp->getOpcode() == llvm::Instruction::FSub) {
          llvm::Value *SubOperand = SubOp->getOperand(1);
          if (auto *MulOp = llvm::dyn_cast<llvm::BinaryOperator>(SubOperand)) {
            if (MulOp->getOpcode() == llvm::Instruction::FMul) {
              MulSubsToReplace.push_back(SubOp);
            }
          }
        }
      }
    }
  }

  for (auto *SubOp : MulSubsToReplace) {
    auto *MulOp = llvm::cast<llvm::BinaryOperator>(SubOp->getOperand(1));
    llvm::Value *A = MulOp->getOperand(0);
    llvm::Value *B = MulOp->getOperand(1);
    llvm::Value *C = SubOp->getOperand(0);

    llvm::IRBuilder<> Builder(SubOp);

    llvm::Value *NegA = Builder.CreateFNeg(A);
    llvm::Value *FMA = Builder.CreateIntrinsic(llvm::Intrinsic::fma,
                                               NegA->getType(), {NegA, B, C});

    SubOp->replaceAllUsesWith(FMA);
    SubOp->eraseFromParent();
    if (MulOp->use_empty()) {
      MulOp->eraseFromParent();
    }
  }
}

struct MulSubOptPass : llvm::PassInfoMixin<MulSubOptPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    visitor(F);
    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

llvm::PassPluginLibraryInfo getMulSubPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MulSubPass", LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                  if (Name == "mulsub-opt-pass") {
                    FPM.addPass(MulSubOptPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMulSubPluginInfo();
}
