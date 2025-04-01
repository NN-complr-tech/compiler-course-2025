#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    struct ExamplePass : PassInfoMixin<ExamplePass> {
        PreservedAnalyses run(Function& F, FunctionAnalysisManager&) {
            Module* M = F.getParent();
            Function* addFunc = nullptr;

            for (Function& Func : M->functions()) {
                if (Func.getName() == "add" && Func.arg_size() == 2) {
                    addFunc = &Func;
                    break;
                }
            }

            if (!addFunc) {
                return PreservedAnalyses::all();
            }

            bool changed = false;

            for (BasicBlock& BB : F) {
                for (Instruction& I : make_early_inc_range(BB)) {
                    if (auto* binOp = dyn_cast<BinaryOperator>(&I)) {
                        if (binOp->getOpcode() == Instruction::Add) {
                            if (addFunc->getFunctionType()->getReturnType() == binOp->getType() &&
                                addFunc->getFunctionType()->getParamType(0) == binOp->getOperand(0)->getType() &&
                                addFunc->getFunctionType()->getParamType(1) == binOp->getOperand(1)->getType()) {
                                IRBuilder<> builder(binOp);
                                Value* call = builder.CreateCall(addFunc, { binOp->getOperand(0), binOp->getOperand(1) });
                                binOp->replaceAllUsesWith(call);
                                binOp->eraseFromParent();
                                changed = true;
                            }
                        }
                    }
                }
            }

            return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
        }

        static bool isRequired() { return true; }
    };
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return { LLVM_PLUGIN_API_VERSION, "ExamplePass", "0.1",
            [](PassBuilder& PB) {
              PB.registerPipelineParsingCallback(
                  [](StringRef Name, FunctionPassManager& FPM,
                     ArrayRef<PassBuilder::PipelineElement>) -> bool {
                    if (Name == "example") {
                      FPM.addPass(ExamplePass{});
                      return true;
                    }
                    return false;
                  });
            } };
}
