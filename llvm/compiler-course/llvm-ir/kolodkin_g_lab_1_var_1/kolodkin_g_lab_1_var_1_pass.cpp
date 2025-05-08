#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <unordered_map>
#include <utility>

namespace {

struct PairHash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2> &pair) const {
    auto hash1 = std::hash<T1>{}(pair.first);
    auto hash2 = std::hash<T2>{}(pair.second);
    return hash1 ^ hash2;
  }
};

struct PairEqual {
  template <class T1, class T2>
  bool operator()(const std::pair<T1, T2> &lhs,
                  const std::pair<T1, T2> &rhs) const {
    return lhs.first == rhs.first && lhs.second == rhs.second;
  }
};

class FusedMultiplyAddPass : public llvm::PassInfoMixin<FusedMultiplyAddPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &) {
    bool modified = false;

    for (auto &BB : F) {
      std::unordered_map<std::pair<llvm::Value *, llvm::Value *>, llvm::Value *,
                         PairHash, PairEqual>
          seenMultiplications;

      for (auto &Inst : llvm::make_early_inc_range(BB)) {
        auto *FAdd = llvm::dyn_cast<llvm::BinaryOperator>(&Inst);
        if (!FAdd || FAdd->getOpcode() != llvm::Instruction::FAdd) {
          continue;
        }

        for (int idx = 0; idx < 2; ++idx) {
          auto *FMul =
              llvm::dyn_cast<llvm::BinaryOperator>(FAdd->getOperand(idx));
          if (!FMul || FMul->getOpcode() != llvm::Instruction::FMul) {
            continue;
          }

          bool isUsedInDivision = false;
          for (auto *U : FAdd->users()) {
            if (auto *Div = llvm::dyn_cast<llvm::BinaryOperator>(U)) {
              if (Div->getOpcode() == llvm::Instruction::FDiv) {
                isUsedInDivision = true;
                break;
              }
            }
          }

          auto key = std::make_pair(FMul->getOperand(0), FMul->getOperand(1));
          if (seenMultiplications.find(key) != seenMultiplications.end()) {
            auto *existingMulResult = seenMultiplications[key];
            FAdd->replaceAllUsesWith(existingMulResult);
            FAdd->eraseFromParent();
            modified = true;
            break;
          } else {
            seenMultiplications[key] = FMul;
          }

          if (!isUsedInDivision) {
            llvm::IRBuilder<> Builder(FAdd);
            auto *fmaInst = Builder.CreateIntrinsic(
                llvm::Intrinsic::fmuladd, {FMul->getType()},
                {FMul->getOperand(0), FMul->getOperand(1),
                 FAdd->getOperand(1 - idx)});
            FAdd->replaceAllUsesWith(fmaInst);
            FAdd->eraseFromParent();

            if (FMul->use_empty()) {
              FMul->eraseFromParent();
            }

            modified = true;
          }
          break;
        }
      }
    }

    return modified ? llvm::PreservedAnalyses::none()
                    : llvm::PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FusedMultuplyAddPass", "0.1",
          [](llvm::PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) -> bool {
                  if (name == "add-and-mul-fma") {
                    FPM.addPass(FusedMultiplyAddPass{});
                    return true;
                  }
                  return false;
                });
          }};
}
