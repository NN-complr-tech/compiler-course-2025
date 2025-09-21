#include "llvm/IR/PassManager.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace llvm::PatternMatch;

namespace {
struct Mul2ToAddPass : PassInfoMixin<Mul2ToAddPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    bool Changed = false;
    for (auto &BB : F) {
      for (auto It = BB.begin(), E = BB.end(); It != E; ) {
        Instruction *I = &*It++;
        Value *X = nullptr;
        // match: mul X, 2
        if (match(I, m_Mul(m_Value(X), m_SpecificInt(2))) ||
            match(I, m_Mul(m_SpecificInt(2), m_Value(X)))) {
          auto *Add = BinaryOperator::CreateAdd(X, X, "", I);
          I->replaceAllUsesWith(Add);
          I->eraseFromParent();
          Changed = true;
        }
      }
    }
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};
} // namespace

// Registration for 'opt -passes=mul2toadd'
llvm::PassPluginLibraryInfo getMul2ToAddPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CompilerCourseMul2ToAdd", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
              [](StringRef Name, FunctionPassManager &FPM,
                 ArrayRef<PassBuilder::PipelineElement>) {
                if (Name == "mul2toadd") {
                  FPM.addPass(Mul2ToAddPass());
                  return true;
                }
                return false;
              });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMul2ToAddPassPluginInfo();
}
