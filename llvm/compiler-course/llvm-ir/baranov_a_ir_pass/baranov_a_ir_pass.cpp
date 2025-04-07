#include "llvm/ADT/SmallSet.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

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

llvm::SmallVector<FmaCandidate> collectCandidates(llvm::BasicBlock &BB) {
  llvm::SmallVector<FmaCandidate> Candidates;
  llvm::SmallSet<llvm::Value *, 8> UsedFMuls;

  auto isSafeOperand = [](llvm::Value *V) {
    return !llvm::isa<llvm::CastInst>(V);
  };

  for (llvm::Instruction &Inst : BB) {
    if (auto *FAddInst = llvm::dyn_cast<llvm::BinaryOperator>(&Inst)) {
      if (FAddInst->getOpcode() != llvm::Instruction::FAdd)
        continue;

      llvm::Value *Op0 = FAddInst->getOperand(0);
      llvm::Value *Op1 = FAddInst->getOperand(1);

      if (llvm::BinaryOperator *FMulInst = getFMulCandidate(Op0)) {
        llvm::Value *Other = Op1;
        if (FMulInst->getType() == FMulInst->getOperand(0)->getType() &&
            FMulInst->getType() == FMulInst->getOperand(1)->getType() &&
            FMulInst->getType() == Other->getType() &&
            isSafeOperand(FMulInst->getOperand(0)) &&
            isSafeOperand(FMulInst->getOperand(1)) && isSafeOperand(Other) &&
            UsedFMuls.insert(FMulInst).second) {
          Candidates.push_back({FAddInst, FMulInst, Other});
        }
      } else if (llvm::BinaryOperator *FMulInst = getFMulCandidate(Op1)) {
        llvm::Value *Other = Op0;
        if (FMulInst->getType() == FMulInst->getOperand(0)->getType() &&
            FMulInst->getType() == FMulInst->getOperand(1)->getType() &&
            FMulInst->getType() == Other->getType() &&
            isSafeOperand(FMulInst->getOperand(0)) &&
            isSafeOperand(FMulInst->getOperand(1)) && isSafeOperand(Other) &&
            UsedFMuls.insert(FMulInst).second) {
          Candidates.push_back({FAddInst, FMulInst, Other});
        }
      }
    }
  }

  return Candidates;
}

void replaceCandidate(const FmaCandidate &Candidate,
                      llvm::SmallVector<llvm::Instruction *> &ToErase) {
  llvm::IRBuilder<> Builder(Candidate.FAddInst);

  auto *FMul = Candidate.FMulInst;
  auto *FAdd = Candidate.FAddInst;
  auto *FMAValue = Builder.CreateIntrinsic(
      llvm::Intrinsic::fmuladd, FMul->getType(),
      {FMul->getOperand(0), FMul->getOperand(1), Candidate.OtherOperand});

  // Заменяем все использования FAdd на FMA
  FAdd->replaceAllUsesWith(FMAValue);
  ToErase.push_back(FAdd);

  // Удаляем FMul, если оно больше нигде не используется (до замены!)
  if (FMul->hasOneUse()) {
    ToErase.push_back(FMul);
  }
}

struct FusedMulAddPass : llvm::PassInfoMixin<FusedMulAddPass> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &) {
    bool Changed = false;
    llvm::SmallVector<llvm::Instruction *> ToErase;

    for (llvm::BasicBlock &BB : Func) {
      llvm::SmallVector<FmaCandidate> Candidates = collectCandidates(BB);

      for (const FmaCandidate &Candidate : Candidates) {
        replaceCandidate(Candidate, ToErase);
        Changed = true;
      }
    }

    for (llvm::Instruction *Inst : ToErase) {
      if (llvm::isa<llvm::BinaryOperator>(Inst) &&
          Inst->getOpcode() == llvm::Instruction::FAdd) {
        Inst->eraseFromParent();
      }
    }

    for (llvm::Instruction *Inst : ToErase) {
      if (llvm::isa<llvm::BinaryOperator>(Inst) &&
          Inst->getOpcode() == llvm::Instruction::FMul) {
        if (Inst->use_empty()) {
          Inst->eraseFromParent();
        }
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
