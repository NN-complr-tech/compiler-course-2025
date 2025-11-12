#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct VectorCounterPass : public PassInfoMixin<VectorCounterPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    LLVMContext &Context = F.getContext();
    Module *M = F.getParent();

    GlobalVariable *Counter = M->getNamedGlobal("vector_instructions_counter");
    if (!Counter) {
      Counter = new GlobalVariable(
          *M, Type::getInt64Ty(Context), false, GlobalValue::ExternalLinkage,
          ConstantInt::get(Type::getInt64Ty(Context), 0),
          "vector_instructions_counter");
    }

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (I.getType()->isVectorTy()) {
          IRBuilder<> Builder(&I);

          Value *OldCount =
              Builder.CreateLoad(Type::getInt64Ty(Context), Counter);
          Value *NewCount = Builder.CreateAdd(OldCount, Builder.getInt64(1));
          Builder.CreateStore(NewCount, Counter);
        }
      }
    }

    return PreservedAnalyses::all();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION,
          "VectorCounterPass_IvashchukVA_FIIT2_BACKEND", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "vector-counter") {
                    FPM.addPass(VectorCounterPass());
                    return true;
                  }
                  return false;
                });
          }};
}