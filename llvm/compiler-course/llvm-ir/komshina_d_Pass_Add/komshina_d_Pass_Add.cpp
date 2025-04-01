#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
    struct ReplaceAddPass : llvm::PassInfoMixin<ReplaceAddPass> {
        llvm::PreservedAnalyses run(llvm::Function& F,
            llvm::FunctionAnalysisManager&) {
            llvm::Module* M = F.getParent();
            llvm::Function* addFunction = M->getFunction("add");

            if (!addFunction || addFunction->arg_size() != 2) {
                return llvm::PreservedAnalyses::all();
            }

            bool changed = false;
            for (llvm::BasicBlock& BB : F) {
                for (llvm::Instruction& I : llvm::make_early_inc_range(BB)) {
                    if (auto* binOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
                        if (binOp->getOpcode() == llvm::Instruction::Add) {
                            llvm::Type* opType = binOp->getType();
                            if (addFunction->getFunctionType()->getReturnType() == opType &&
                                addFunction->getArg(0)->getType() == binOp->getOperand(0)->getType() &&
                                addFunction->getArg(1)->getType() == binOp->getOperand(1)->getType()) {
                                llvm::IRBuilder<> builder(binOp);
                                llvm::Value* call = builder.CreateCall(addFunction, { binOp->getOperand(0), binOp->getOperand(1) });
                                binOp->replaceAllUsesWith(call);
                                binOp->eraseFromParent();
                                changed = true;
                            }
                        }
                    }
                }
            }
            return changed ? llvm::PreservedAnalyses::none()
                : llvm::PreservedAnalyses::all();
        }

        static bool isRequired() { return true; }
    };
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return { LLVM_PLUGIN_API_VERSION, "ReplaceAddPass", "0.1",
            [](llvm::PassBuilder& PB) {
              PB.registerPipelineParsingCallback(
                  [](llvm::StringRef name, llvm::FunctionPassManager& FPM,
                     llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                    if (name == "ReplaceAddPass") {
                      FPM.addPass(ReplaceAddPass{});
                      return true;
                    }
                    return false;
                  });
            } };
}