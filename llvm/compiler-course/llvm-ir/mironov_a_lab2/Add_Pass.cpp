#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

using namespace std;
using namespace llvm;

namespace {
struct Add_Pass : llvm::PassInfoMixin<Add_Pass> {
  llvm::PreservedAnalyses run(llvm::Module &module,
                              llvm::ModuleAnalysisManager &) {

    pair<Type *, Type *> add_types;
    Function * add_func_ptr = nullptr;
    bool none = 0;

    // store all overloads
    for (auto &func : module) {

      if (func.getName() != "add") { 
        continue;
      }
      
      FunctionType *funcType = func.getFunctionType();
      if (funcType->getNumParams() == 2) {
        add_types = {funcType->getParamType(0), funcType->getParamType(1)};
        add_func_ptr = &func;
        break;
      }
    }
    if (add_func_ptr == nullptr){
      return PreservedAnalyses::all();
    }

    // check all add operators
    for (auto &func : module) {

      if (&func == add_func_ptr) {
        continue;
      }
  
      for (auto &op : func) {
        for (auto it = op.begin(); it != op.end(); ) {
          auto &inst = *it++;
          if (isa<BinaryOperator>(inst) && inst.getOpcode() == Instruction::Add) {
            Type *left_type = inst.getOperand(0)->getType();
            Type *right_type = inst.getOperand(1)->getType();
            pair<Type *, Type *> par = {left_type, right_type};

            if (par == add_types) {
              IRBuilder<> builder(&inst);
              Value *call = builder.CreateCall(add_func_ptr, {inst.getOperand(0), inst.getOperand(1)});
              inst.replaceAllUsesWith(call);
              inst.eraseFromParent();
              none=true;
            }
          }
        }
      }
    }

    if (none){
      return PreservedAnalyses::none();
    }
    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "Add_Pass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::ModulePassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "Add_Pass") {
                    FPM.addPass(Add_Pass{});
                    return true;
                  }
                  return false;
                });
          }};
}
