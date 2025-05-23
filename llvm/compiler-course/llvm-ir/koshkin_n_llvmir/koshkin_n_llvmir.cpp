#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct PureFuncPass : llvm::PassInfoMixin<PureFuncPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    using namespace llvm;

    if (func.hasFnAttribute(Attribute::ReadNone) ||
        func.hasFnAttribute(Attribute::ReadOnly) ||
        func.hasFnAttribute("pure")) {
      return PreservedAnalyses::all();
    }

    bool isPure = true;
    for (Instruction &instruct : instructions(func)) {
      if (instruct.mayReadOrWriteMemory()) {
        isPure = false;
        break;
      }
    }

    if (isPure)
      func.addFnAttr("pure");

    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PureFuncPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "mark_pure_func_pass") {
                    FPM.addPass(PureFuncPass{});
                    return true;
                  }
                  return false;
                });
          }};
}