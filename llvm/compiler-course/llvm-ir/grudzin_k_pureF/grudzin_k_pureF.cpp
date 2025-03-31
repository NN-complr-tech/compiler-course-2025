#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

namespace {
struct MarkPurePass : llvm::PassInfoMixin<MarkPurePass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {

    if (Func.hasFnAttribute("pure")) {
      return llvm::PreservedAnalyses::all();
    }

    // Use inst iterators from thi ref:
    // https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/IR/InstIterator.h
    bool Pure = std::none_of(llvm::inst_begin(Func), llvm::inst_end(Func),
                             [](const llvm::Instruction &Instruction) {
                               return Instruction.mayReadOrWriteMemory();
                             });

    if (Pure) {
      Func.addFnAttr("pure");
    }
    return llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MarkPurePass", "1.0",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "mark-pure-pass") {
                    FPM.addPass(MarkPurePass{});
                    return true;
                  }
                  return false;
                });
          }};
}
