#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

/*
Impure functions:
1. return value variation with a static variable
2. return value variation with a non-local variable
3. return value variation with a mutable reference argument
4. return value variation with an input stream
5. mutation of a local static variable
6. mutation of a non-local variable
7. mutation of a mutable reference argument
8. mutation of an output stream

Tests: https://compiler-explorer.com/z/xxbnvnaj8

If function is empty, I assume it's pure, because it has no side effects
*/

namespace {
struct PureFunctionPass : llvm::PassInfoMixin<PureFunctionPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    if (!(staticOrGlobalVariable(func) || referenceArgument(func) ||
          inputOutput(func))) {
      if (!func.hasFnAttribute("pure"))
        func.addFnAttr("pure");
    }

    return llvm::PreservedAnalyses::all();
  }

  // Case: 1, 2, 5, 6.
  bool staticOrGlobalVariable(llvm::Function &func) {
    for (llvm::BasicBlock &block : func) {
      for (llvm::Instruction &i : block) {
        llvm::Value *ptr = nullptr;
        if (auto *load = llvm::dyn_cast<llvm::LoadInst>(&i))
          ptr = load->getPointerOperand()->stripPointerCasts();
        else if (auto *store = llvm::dyn_cast<llvm::StoreInst>(&i))
          ptr = store->getPointerOperand()->stripPointerCasts();
        if (ptr != nullptr) {
          auto *var = llvm::dyn_cast<llvm::GlobalVariable>(ptr);
          if (var && !var->isConstant()) {
            return true;
          }
        }
      }
    }
    return false;
  }

  // Case: 3, 7.
  bool referenceArgument(llvm::Function &func) {
    for (llvm::Argument &arg : func.args()) {
      if (arg.getType()->isPointerTy() &&
          !(arg.hasAttribute(llvm::Attribute::ReadOnly)))
        return true;
    }
    return false;
  }

  // Case: 4, 8 (too much IO functions, I'll make only std::cin, std::cout)
  bool inputOutput(llvm::Function &func) {
    for (llvm::BasicBlock &block : func) {
      for (llvm::Instruction &i : block) {
        if (auto *call = llvm::dyn_cast<llvm::CallInst>(&i)) {
          for (auto &arg : call->args()) {
            if (auto *var = llvm::dyn_cast<llvm::GlobalVariable>(arg)) {
              llvm::StringRef name = var->getName();
              if (name.contains("cin") || name.contains("cout"))
                return true;
            }
          }
        }
      }
    }
    return false;
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFunctionPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "purefunctionpass") {
                    FPM.addPass(PureFunctionPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
