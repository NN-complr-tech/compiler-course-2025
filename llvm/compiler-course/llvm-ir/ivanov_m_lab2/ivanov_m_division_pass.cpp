#include <cmath>
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct BitShiftPass : llvm::PassInfoMixin<BitShiftPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool isModified = false;

    for (llvm::BasicBlock &BB : func) {
      for (auto It = BB.begin(), End = BB.end(); It != End; ) {
        llvm::Instruction *Inst = &*It++;
        
        auto *DivInst = llvm::dyn_cast<llvm::BinaryOperator>(Inst);

        if (DivInst) {
          // check if it's not division operator
          if (!(DivInst->getOpcode() == llvm::Instruction::SDiv ||
          DivInst->getOpcode() == llvm::Instruction::UDiv))
            continue;
          
          // check if right operator is constant
          auto *ConstInt = llvm::dyn_cast<llvm::ConstantInt>(DivInst->getOperand(1));
          if (ConstInt) {
            llvm::IRBuilder<> Builder(DivInst);
  
            // receiving signed value of right operator
            int64_t Divisor = ConstInt->getSExtValue();
            bool isNegDivisor = Divisor < 0;
            // receiving absolute value of right operator
            uint64_t AbsDivisor = static_cast<uint64_t>(std::llabs(Divisor));

            // check if right operator is '0'
            if (Divisor == 0 || !llvm::isPowerOf2_64(AbsDivisor))
              continue;

            // when right operator is '1' or '-1'
            if (DivInst->getOpcode() == llvm::Instruction::SDiv && AbsDivisor == 1) {
              if(Divisor == -1){
                llvm::Value *Neg = Builder.CreateNeg(DivInst->getOperand(0), "neg");
                DivInst->replaceAllUsesWith(Neg);
              } else {
                DivInst->replaceAllUsesWith(DivInst->getOperand(0));
              }
              DivInst->eraseFromParent();
              isModified = true;
              continue;
            }
            
            // shift counting
            unsigned ShiftAmount = llvm::Log2_64(AbsDivisor);
            
            llvm::Value *NewInst = nullptr;

            if (DivInst->getOpcode() == llvm::Instruction::SDiv) {
              NewInst = Builder.CreateAShr(
                  DivInst->getOperand(0),
                  llvm::ConstantInt::get(DivInst->getOperand(0)->getType(), ShiftAmount),
                  "ashr");
              if (isNegDivisor)
                NewInst = Builder.CreateNeg(NewInst, "neg");
            } else {
              NewInst = Builder.CreateLShr(
                  DivInst->getOperand(0),
                  llvm::ConstantInt::get(DivInst->getOperand(0)->getType(), ShiftAmount),
                  "lshr");
            }

            DivInst->replaceAllUsesWith(NewInst);
            DivInst->eraseFromParent();
            isModified = true;
          }
        }
      }
    }

    return isModified ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "BitShiftPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "bit_shift_pass") {
                    FPM.addPass(BitShiftPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
