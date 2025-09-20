#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"

namespace {
struct FMAPass : llvm::PassInfoMixin<FMAPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool transformed = false;

    for (llvm::BasicBlock &bbIt : F) {
      for (llvm::Instruction &instIt : llvm::make_early_inc_range(bbIt)) {
        llvm::Instruction *Inst = &instIt;
        if (tryFuseAt(Inst)) {
          transformed = true;
        }
      }
    }

    return transformed ? llvm::PreservedAnalyses::none()
                       : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }

private:
  static bool tryFuseAt(llvm::Instruction *I) {
    auto *AddBO = llvm::dyn_cast<llvm::BinaryOperator>(I);
    if (!AddBO) {
      return false;
    }

    if (AddBO->getOpcode() != llvm::Instruction::FAdd) {
      return false;
    }

    unsigned numOps = AddBO->getNumOperands();
    for (unsigned idx = 0; idx < numOps; ++idx) {
      llvm::Value *op = AddBO->getOperand(idx);
      auto *MulBO = llvm::dyn_cast<llvm::BinaryOperator>(op);
      if (!MulBO) {
        continue;
      }

      if (MulBO->getOpcode() != llvm::Instruction::FMul) {
        continue;
      }

      if (!MulBO->hasOneUse()) {
        continue;
      }

      llvm::Value *other = AddBO->getOperand(1u - idx);

      llvm::Module *M = AddBO->getModule();
      if (!M) {
        return false;
      }

      auto createFMA = [&](llvm::BinaryOperator *Mul,
                           llvm::Value *Addend) -> llvm::Value * {
        llvm::SmallVector<llvm::Value *, 3> args;
        args.push_back(Mul->getOperand(0));
        args.push_back(Mul->getOperand(1));
        args.push_back(Addend);
        llvm::Function *decl = llvm::Intrinsic::getDeclaration(
            M, llvm::Intrinsic::fmuladd, Mul->getType());
        llvm::CallInst *call = llvm::CallInst::Create(decl, args, "fma", AddBO);
        return call;
      };

      llvm::Value *call = createFMA(MulBO, other);

      AddBO->replaceAllUsesWith(call);

      AddBO->eraseFromParent();
      if (MulBO->use_empty()) {
        MulBO->eraseFromParent();
      }

      return true;
    }

    return false;
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMAPass", "0.1", [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "fmapass") {
                    FPM.addPass(FMAPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
