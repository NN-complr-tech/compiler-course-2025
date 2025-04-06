#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct Add_Pass : llvm::PassInfoMixin<Add_Pass> {
  llvm::PreservedAnalyses run(llvm::Module &module,
                              llvm::ModuleAnalysisManager &) {

    std::pair<llvm::Type *, llvm::Type *> add_types;
    llvm::Function *add_func_ptr = nullptr;
    bool none = 0;

    // store all overloads
    for (auto &func : module) {

      if (func.getName() != "add") {
        continue;
      }

      llvm::FunctionType *funcType = func.getFunctionType();
      if (funcType->getNumParams() == 2) {
        add_types = {funcType->getParamType(0), funcType->getParamType(1)};
        add_func_ptr = &func;
        break;
      }
    }
    if (add_func_ptr == nullptr) {
      return llvm::PreservedAnalyses::all();
    }

    // check all add operators
    for (auto &func : module) {

      if (&func == add_func_ptr) {
        continue;
      }

      for (auto &op : func) {
        for (auto it = op.begin(); it != op.end();) {
          auto &inst = *it++;
          if (llvm::isa<llvm::BinaryOperator>(inst) &&
              inst.getOpcode() == llvm::Instruction::Add) {
            llvm::Type *left_type = inst.getOperand(0)->getType();
            llvm::Type *right_type = inst.getOperand(1)->getType();
            std::pair<llvm::Type *, llvm::Type *> par = {left_type, right_type};

            if (par == add_types) {
              llvm::IRBuilder<> builder(&inst);
              llvm::Value *call = builder.CreateCall(
                  add_func_ptr, {inst.getOperand(0), inst.getOperand(1)});
              inst.replaceAllUsesWith(call);
              inst.eraseFromParent();
              none = true;
            }
          }
        }
      }
    }

    if (none) {
      return llvm::PreservedAnalyses::none();
    }
    return llvm::PreservedAnalyses::all();
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
