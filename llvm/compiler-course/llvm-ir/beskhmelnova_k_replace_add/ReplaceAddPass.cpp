#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <map>

namespace {
struct ReplaceAddPass : llvm::PassInfoMixin<ReplaceAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    if (F.getName() == "add" || F.getName() == "sub" || F.getName() == "mul" ||
        F.getName() == "div") {
      return llvm::PreservedAnalyses::all();
    }

    bool changed = false;
    llvm::Module *M = F.getParent();

    std::map<unsigned, std::string> opToFunc = {
        {llvm::Instruction::Add, "add"},
        {llvm::Instruction::Sub, "sub"},
        {llvm::Instruction::Mul, "mul"},
        {llvm::Instruction::SDiv, "div"}};

    std::map<unsigned, llvm::Function *> funcMap;
    for (llvm::Function &Func : M->functions()) {
      for (const auto &[opcode, funcName] : opToFunc) {
        if (Func.getName().contains(funcName) && Func.arg_size() == 2) {
          funcMap[opcode] = &Func;
        }
      }
    }

    if (funcMap.empty()) {
      return llvm::PreservedAnalyses::all();
    }

    for (llvm::BasicBlock &BB : F) {
      for (auto It = BB.begin(); It != BB.end();) {
        llvm::Instruction &I = *It++;

        if (auto *binOp = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
          auto it = funcMap.find(binOp->getOpcode());
          if (it != funcMap.end()) {
            llvm::Function *replacementFunction = it->second;
            llvm::IRBuilder<> builder(binOp);

            llvm::Value *call =
                builder.CreateCall(replacementFunction, {binOp->getOperand(0),
                                                         binOp->getOperand(1)});

            call->setName(binOp->getName());
            binOp->replaceAllUsesWith(call);
            binOp->eraseFromParent();
            changed = true;
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
  return {LLVM_PLUGIN_API_VERSION, "ReplaceAddPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "ReplaceAddPass") {
                    FPM.addPass(ReplaceAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
