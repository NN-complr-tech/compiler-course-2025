#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
using namespace llvm;

namespace {
class PureFunctionPass : public PassInfoMixin<PureFunctionPass> {
public:
  PreservedAnalyses run(Function &Foo, FunctionAnalysisManager &) {
    if (Foo.isDeclaration() || Foo.hasFnAttribute(Attribute::ReadNone) ||
        Foo.hasFnAttribute(Attribute::ReadOnly) ||
        Foo.hasFnAttribute("pure")) // Skip function declarations and already
                                    // known pure/read-only functions
      return PreservedAnalyses::all();

    if (!FoundSideEffects(Foo)) {
      Foo.addFnAttr("pure");
      return PreservedAnalyses::none(); // Indicate that pass has modified the
                                        // function
    }
    return PreservedAnalyses::all();
  }

  bool FoundSideEffects(Function &Foo) {
    for (auto &BaseBlock : Foo) {
      for (auto &Instruction : BaseBlock) {
        if (isa<StoreInst>(Instruction) || isa<AtomicRMWInst>(Instruction) ||
            isa<AtomicCmpXchgInst>(
                Instruction)) { // Any memory write makes the function impure
          return true;
        }

        if (auto *CI = dyn_cast<CallBase>(&Instruction)) {
          Function *Callee = CI->getCalledFunction();
          if (!Callee) {
            return true;
          }

          if (Callee &&
              !(Callee->hasFnAttribute(Attribute::ReadOnly) ||
                Callee->hasFnAttribute("pure") ||
                Callee->hasFnAttribute(
                    Attribute::ReadNone))) { // If the called function is
                                             // unknown or not pure/readonly,
                                             // the current function as also
                                             // impure
            return true;
          }
        }

        if (auto *LdInst = dyn_cast<LoadInst>(&Instruction)) {
          if (LdInst->isVolatile()) {
            return true;
          }

          if (auto *GV = dyn_cast<GlobalVariable>(LdInst->getPointerOperand())) {
            if (!GV->isConstant()) {
              return true;
            }
          }
        }

        if (isa<ResumeInst>(Instruction) || isa<CatchSwitchInst>(Instruction) ||
            isa<CatchPadInst>(Instruction)) {
          return true; // These instructions can affect the program's state
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
