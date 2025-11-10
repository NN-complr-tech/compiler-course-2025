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
    // Create or get the global counter variable
    GlobalVariable *VectorCounter =
        M.getNamedGlobal("vector_instructions_counter");
    if (!VectorCounter) {
      VectorCounter = new GlobalVariable(
          M, IntegerType::getInt64Ty(M.getContext()), false,
          GlobalValue::ExternalLinkage,
          ConstantInt::get(IntegerType::getInt64Ty(M.getContext()), 0),
          "vector_instructions_counter");
    }

    Function *IncrementFunc = M.getFunction("increment_vector_counter");
    if (!IncrementFunc) {
      FunctionType *FT =
          FunctionType::get(Type::getVoidTy(M.getContext()), false);
      IncrementFunc = Function::Create(FT, GlobalValue::ExternalLinkage,
                                       "increment_vector_counter", &M);
    }

    bool Modified = false;
    IRBuilder<> Builder(M.getContext());

    for (auto &F : M) {
      if (F.isDeclaration())
        continue;

      for (auto &BB : F) {
        for (auto &I : BB) {
          if (isVectorInstruction(&I)) {
            // Insert counter increment after the vector instruction
            Builder.SetInsertPoint(&BB, ++I.getIterator());

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

    // Add print function to output the counter value
    addPrintFunction(M, VectorCounter);

    return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

private:
  bool isVectorInstruction(Instruction *I) {

    for (auto &Op : I->operands()) {
      if (Op->getType()->isVectorTy()) {
        return true;
      }
    }

    if (I->getType()->isVectorTy()) {
      return true;
    }

    if (auto *CI = dyn_cast<CallInst>(I)) {
      if (Function *F = CI->getCalledFunction()) {
        StringRef Name = F->getName();
        if (Name.startswith("llvm.") &&
            (Name.contains("vector") || Name.contains("simd") ||
             Name.contains("mask") || Name.contains("gather") ||
             Name.contains("scatter"))) {
          return true;
        }
      }
    }

    return false;
  }

  void addPrintFunction(Module &M, GlobalVariable *Counter) {
    FunctionType *PrintfType =
        FunctionType::get(IntegerType::getInt32Ty(M.getContext()),
                          PointerType::getUnqual(M.getContext()), true);
    Function *PrintfFunc = M.getOrInsertFunction("printf", PrintfType);

    FunctionType *FT =
        FunctionType::get(Type::getVoidTy(M.getContext()), false);
    Function *PrintCounterFunc = Function::Create(
        FT, GlobalValue::ExternalLinkage, "print_vector_counter", &M);

    BasicBlock *BB =
        BasicBlock::Create(M.getContext(), "entry", PrintCounterFunc);
    IRBuilder<> Builder(BB);

    Constant *FormatStr = Builder.CreateGlobalStringPtr(
        "Total vector instructions executed: %lld\n");

    LoadInst *CounterVal =
        Builder.CreateLoad(IntegerType::getInt64Ty(M.getContext()), Counter);

    Builder.CreateCall(PrintfFunc, {FormatStr, CounterVal});
    Builder.CreateRetVoid();
  }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION,
          "IvashchukVA Lab3 - Vector Instructions Counter", LLVM_VERSION_STRING,
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