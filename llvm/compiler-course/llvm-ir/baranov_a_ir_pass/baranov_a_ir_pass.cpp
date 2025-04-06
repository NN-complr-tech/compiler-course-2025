#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <vector>

namespace {

struct FmaCandidate {
  llvm::BinaryOperator *FAddInst;
  llvm::BinaryOperator *FMulInst;
  llvm::Value *OtherOperand;
};

llvm::BinaryOperator *getFMulCandidate(llvm::Value *Operand) {
  if (auto *OpInst = llvm::dyn_cast<llvm::BinaryOperator>(Operand)) {
    if (OpInst->getOpcode() == llvm::Instruction::FMul)
      return OpInst;
  }
  return nullptr;
}

std::vector<FmaCandidate> collectCandidates(llvm::BasicBlock &BB) {
  std::vector<FmaCandidate> Candidates;

  for (llvm::Instruction &Inst : BB) {
    if (auto *FAddInst = llvm::dyn_cast<llvm::BinaryOperator>(&Inst)) {
      if (FAddInst->getOpcode() != llvm::Instruction::FAdd)
        continue;
      llvm::Value *Op0 = FAddInst->getOperand(0);
      llvm::Value *Op1 = FAddInst->getOperand(1);

      if (llvm::BinaryOperator *FMulInst = getFMulCandidate(Op0)) {
        Candidates.push_back({FAddInst, FMulInst, Op1});
      } else if (llvm::BinaryOperator *FMulInst = getFMulCandidate(Op1)) {
        Candidates.push_back({FAddInst, FMulInst, Op0});
      }
    }
  }

  return Candidates;
}

void replaceCandidate(const FmaCandidate &Candidate,
                      std::vector<llvm::Instruction *> &ToErase) {
  llvm::IRBuilder<> Builder(Candidate.FAddInst);

  llvm::Value *FMAValue = Builder.CreateIntrinsic(
      llvm::Intrinsic::fmuladd, Candidate.FMulInst->getType(),
      {Candidate.FMulInst->getOperand(0), Candidate.FMulInst->getOperand(1),
       Candidate.OtherOperand});

  Candidate.FAddInst->replaceAllUsesWith(FMAValue);

  ToErase.push_back(Candidate.FAddInst);

  if (Candidate.FMulInst->use_empty()) {
    ToErase.push_back(Candidate.FMulInst);
  }
}

struct FusedMulAddPass : llvm::PassInfoMixin<FusedMulAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    std::vector<llvm::Instruction *> ToErase;

    for (llvm::BasicBlock &BB : Func) {
      std::vector<FmaCandidate> Candidates = collectCandidates(BB);

      for (const FmaCandidate &Candidate : Candidates) {
        replaceCandidate(Candidate, ToErase);
        Changed = true;
      }
    }

    for (llvm::Instruction *Inst : ToErase) {
      if (Inst->use_empty()) {
        Inst->eraseFromParent();
      }
    }

    llvm::verifyFunction(Func);
    return Changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FMulAddPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (Name == "FMulAddPass") {
                    FPM.addPass(FusedMulAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}