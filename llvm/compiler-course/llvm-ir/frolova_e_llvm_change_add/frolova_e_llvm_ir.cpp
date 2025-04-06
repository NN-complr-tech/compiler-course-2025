#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
struct ChangeADD : llvm::PassInfoMixin<ChangeADD> {
private:
  bool changed = false;

  struct ReplaceInfo {
    unsigned opcode;
    llvm::Function *func;
  };

  void
  processBasicBlock(llvm::BasicBlock &BB,
                    const llvm::SmallVector<ReplaceInfo, 4> &replacements) {
    for (auto &instruction : llvm::make_early_inc_range(BB)) {
      if (auto *BI = llvm::dyn_cast<llvm::BinaryOperator>(&instruction)) {
        for (const auto &rep : replacements) {
          if (BI->getOpcode() == rep.opcode && rep.func) {
            llvm::FunctionType *funcType = rep.func->getFunctionType();
            llvm::Type *operand0Type = BI->getOperand(0)->getType();
            llvm::Type *operand1Type = BI->getOperand(1)->getType();
            llvm::Type *resultType = BI->getType();

            if (funcType->getNumParams() == 2 &&
                funcType->getParamType(0) == operand0Type &&
                funcType->getParamType(1) == operand1Type &&
                funcType->getReturnType() == resultType) {

              replaceAddWithCall(BI, rep.func);
              BI->eraseFromParent();
              changed = true;
              break;
            }
          }
        }
      }
    }
  }

  void replaceAddWithCall(llvm::BinaryOperator *BI, llvm::Function *addFunc) {
    llvm::IRBuilder<> Builder(BI);
    std::string originalName = BI->hasName() ? BI->getName().str() : "";
    llvm::CallInst *Call =
        Builder.CreateCall(addFunc, {BI->getOperand(0), BI->getOperand(1)});
    if (!originalName.empty()) {
      Call->setName(originalName);
    }
    BI->replaceAllUsesWith(Call);
  }

public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    llvm::Module *M = F.getParent();
    llvm::Function *addFunc = M->getFunction("add");
    llvm::Function *faddFunc = M->getFunction("fadd");

    if ((addFunc && F.getName() == "add") ||
        (faddFunc && F.getName() == "fadd")) {
      return llvm::PreservedAnalyses::all();
    }

    llvm::SmallVector<ReplaceInfo, 4> replacements = {
        {llvm::Instruction::Add, addFunc},
        {llvm::Instruction::FAdd, faddFunc},
    };

    for (llvm::BasicBlock &BB : F) {
      processBasicBlock(BB, replacements);
    }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "ChangeADD", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "ChangeADD") {
                    FPM.addPass(ChangeADD{});
                    return true;
                  }
                  return false;
                });
          }};
}