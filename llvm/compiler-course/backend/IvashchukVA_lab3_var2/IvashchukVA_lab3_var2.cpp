#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct VectorCounterPass : public PassInfoMixin<VectorCounterPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
    GlobalVariable *VectorCounter =
        M.getNamedGlobal("vector_instructions_counter");
    if (!VectorCounter) {
      VectorCounter = new GlobalVariable(
          M, IntegerType::getInt64Ty(M.getContext()), false,
          GlobalValue::ExternalLinkage,
          ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0),
          "vector_instructions_counter");
    }

    bool Modified = false;
    IRBuilder<> Builder(M.getContext());

    for (auto &F : M) {
      if (F.isDeclaration())
        continue;

      for (auto &BB : F) {
        for (auto I = BB.begin(); I != BB.end();) {
          Instruction *CurrentI = &*I;
          ++I;

          if (isVectorInstruction(CurrentI)) {
            Builder.SetInsertPoint(&BB, I);

            LoadInst *Load = Builder.CreateLoad(
                IntegerType::getInt64Ty(M.getContext()), VectorCounter);

            Value *Increment = Builder.CreateAdd(
                Load,
                ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 1));

            Builder.CreateStore(Increment, VectorCounter);
            Modified = true;
          }
        }
      }
    }

    return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  bool isVectorInstruction(Instruction *I) {
    if (I->getType()->isVectorTy()) {
      return true;
    }

    for (auto &Op : I->operands()) {
      if (Op->getType()->isVectorTy()) {
        return true;
      }
    }

    return false;
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IvashchukVA Lab3", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "vector-counter") {
                    MPM.addPass(VectorCounterPass());
                    return true;
                  }
                  return false;
                });
          }};
}