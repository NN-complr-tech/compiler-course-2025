#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

    class DivToShiftPass : public llvm::PassInfoMixin<DivToShiftPass> {
    public:
        llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
            bool changed = false;
            for (auto& BB : F) {
                for (auto& I : llvm::make_early_inc_range(BB)) {
                    if (auto* SDiv = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
                        if (SDiv->getOpcode() == llvm::Instruction::SDiv) {
                            if (auto* ConstInt = llvm::dyn_cast<llvm::ConstantInt>(SDiv->getOperand(1))) {
                                int64_t divisor = ConstInt->getSExtValue();
                                llvm::IRBuilder<> Builder(SDiv);
                                if (divisor == 1 || divisor == -1) {
                                    llvm::Value* NewVal = Builder.CreateAdd(SDiv->getOperand(0),
                                        llvm::ConstantInt::get(SDiv->getType(), 0), "add_zero");
                                    if (divisor == -1) {
                                        NewVal = Builder.CreateNeg(NewVal, "neg_tmp");
                                    }
                                    SDiv->replaceAllUsesWith(NewVal);
                                    SDiv->eraseFromParent();
                                    changed = true;
                                    continue;
                                }
                                if (divisor != 0 && (abs(divisor) & (abs(divisor) - 1)) == 0) {
                                    int shiftAmount = llvm::Log2_64(abs(divisor));
                                    llvm::Value* Shifted = Builder.CreateAShr(SDiv->getOperand(0), shiftAmount, "sh_tmp");
                                    if (divisor < 0) {
                                        Shifted = Builder.CreateNeg(Shifted, "neg_tmp");
                                    }
                                    SDiv->replaceAllUsesWith(Shifted);
                                    SDiv->eraseFromParent();
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
            return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
        }
    };

} // namespace

extern "C" llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
    return { LLVM_PLUGIN_API_VERSION, "DivToShiftPass", "v0.4",
              [](llvm::PassBuilder& PB) {
                  PB.registerPipelineParsingCallback(
                      [](llvm::StringRef Name, llvm::FunctionPassManager& FPM, llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                          if (Name == "div-to-shift") {
                              FPM.addPass(DivToShiftPass());
                              return true;
                          }
                          return false;
                      });
              } };
}

