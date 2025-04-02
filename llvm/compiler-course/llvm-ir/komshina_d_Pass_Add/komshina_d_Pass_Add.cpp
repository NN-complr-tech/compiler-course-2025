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
            if (F.getName() == "add") {
                return llvm::PreservedAnalyses::all();
            }

            llvm::Function* addFunction = nullptr;
            llvm::Module* M = F.getParent();

            for (llvm::Function& Func : M->functions()) {
                if (Func.getName() == "add" && Func.arg_size() == 2) {
                    addFunction = &Func;
                    break;
                }
            }

            if (!addFunction) {
                return llvm::PreservedAnalyses::all();
            }

            bool modified = false;

            for (llvm::BasicBlock& BB : F) {
                for (auto it = BB.begin(), end = BB.end(); it != end;) {
                    llvm::Instruction* I = &*it++;
                    if (auto* binOp = llvm::dyn_cast<llvm::BinaryOperator>(I)) {
                        if (binOp->getOpcode() == llvm::Instruction::Add &&
                            binOp->getOperand(0)->getType() == addFunction->getArg(0)->getType() &&
                            binOp->getOperand(1)->getType() == addFunction->getArg(1)->getType()) {

                            llvm::IRBuilder<> builder(binOp);
                            llvm::Value* call = builder.CreateCall(addFunction, { binOp->getOperand(0), binOp->getOperand(1) });
                            call->setName(binOp->getName());
                            binOp->replaceAllUsesWith(call);
                            binOp->eraseFromParent();
                            modified = true;
                        }
                    }
                }
            }

            return modified ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
        }

        static bool isRequired() { return true; }
    };
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
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
