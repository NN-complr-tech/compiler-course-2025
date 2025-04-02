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
            if (F.getName() == "add" || F.getName() == "sub") {
                return llvm::PreservedAnalyses::all();
            }

            llvm::Function* addFunction = nullptr;
            llvm::Function* subFunction = nullptr;
            llvm::Module* M = F.getParent();

            for (llvm::Function& Func : M->functions()) {
                if (Func.getName() == "add" && Func.arg_size() == 2) {
                    addFunction = &Func;
                }
                else if (Func.getName() == "sub" && Func.arg_size() == 2) {
                    subFunction = &Func;
                }
            }

            bool changed = false;
            for (llvm::BasicBlock& BB : F) {
                for (llvm::Instruction& I : llvm::make_early_inc_range(BB)) {
                    if (auto* binOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
                        llvm::Function* replacementFunction = nullptr;
                        if (binOp->getOpcode() == llvm::Instruction::Add) {
                            if (addFunction && binOp->getOperand(0)->getType() == addFunction->getArg(0)->getType() &&
                                binOp->getOperand(1)->getType() == addFunction->getArg(1)->getType()) {
                                replacementFunction = addFunction;
                            }
                        }
                        else if (binOp->getOpcode() == llvm::Instruction::Sub) {
                            if (subFunction && binOp->getOperand(0)->getType() == subFunction->getArg(0)->getType() &&
                                binOp->getOperand(1)->getType() == subFunction->getArg(1)->getType()) {
                                replacementFunction = subFunction;
                            }
                        }
                        if (replacementFunction) {
                            llvm::IRBuilder<> builder(binOp);
                            llvm::Value* call = builder.CreateCall(
                                replacementFunction, { binOp->getOperand(0), binOp->getOperand(1) });
                            call->setName(binOp->getName());
                            binOp->replaceAllUsesWith(call);
                            binOp->eraseFromParent();
                            changed = true;
                        }
                    }
                }
            }
            return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
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
