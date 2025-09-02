#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>

namespace {
struct DivisionToShiftPass : llvm::PassInfoMixin<DivisionToShiftPass> {
  llvm::PreservedAnalyses run(llvm::Function &func,
                              llvm::FunctionAnalysisManager &) {
    bool changed = false;
    std::vector<llvm::Instruction*> toRemove;
    
    for (auto &BB : func) {
      for (auto &I : BB) {
        if (auto *divInst = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          if (divInst->getOpcode() == llvm::Instruction::SDiv || 
              divInst->getOpcode() == llvm::Instruction::UDiv) {
            
            llvm::Value *dividend = divInst->getOperand(0);
            llvm::Value *divisor = divInst->getOperand(1);
            
            if (auto *constDivisor = llvm::dyn_cast<llvm::ConstantInt>(divisor)) {
              int64_t divisorValue = constDivisor->getSExtValue();
              
              // Проверяем, является ли делитель положительной степенью двойки
              if (divisorValue > 0 && (divisorValue & (divisorValue - 1)) == 0) {
                unsigned shiftAmount = llvm::Log2_64(divisorValue);
                
                llvm::IRBuilder<> builder(divInst);
                
                llvm::Value *shiftInst = nullptr;
                if (divInst->getOpcode() == llvm::Instruction::SDiv) {
                  shiftInst = builder.CreateAShr(dividend, shiftAmount);
                } else {
                  shiftInst = builder.CreateLShr(dividend, shiftAmount);
                }
                
                divInst->replaceAllUsesWith(shiftInst);
                toRemove.push_back(divInst);
                
                changed = true;
                llvm::outs() << "Replaced division by " << divisorValue 
                            << " with shift by " << shiftAmount << "\n";
              }
              else if (divisorValue == 1) {
                divInst->replaceAllUsesWith(dividend);
                toRemove.push_back(divInst);
                
                changed = true;
                llvm::outs() << "Replaced division by 1 with the operand itself\n";
              }
            }
          }
        }
      }
    }
    
    for (auto *inst : toRemove) {
      if (inst->use_empty()) {
        inst->eraseFromParent();
      }
    }
    
    return changed ? llvm::PreservedAnalyses::none() 
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DivisionToShiftPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "div-to-shift") {
                    FPM.addPass(DivisionToShiftPass{});
                    return true;
                  }
                  return false;
                });
          }};
}