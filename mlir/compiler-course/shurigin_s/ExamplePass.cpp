#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {

int calculateDepthInsideRegion(Region &InputRegion) {
  int MaxDepthFound = 0;

  for (Block &BlockElement : InputRegion) {
    for (Operation &Op : BlockElement.getOperations()) {
      int CurrentOpNestingLevel = 0;

      if (isa<scf::ForOp, scf::WhileOp, affine::AffineForOp, scf::IfOp,
              affine::AffineIfOp>(&Op)) {
        CurrentOpNestingLevel = 1;

        int MaxDepthInOpSubRegions = 0;
        for (Region &SubRegion : Op.getRegions()) {
          int DepthInThisSubRegion = calculateDepthInsideRegion(SubRegion);
          if (DepthInThisSubRegion > MaxDepthInOpSubRegions) {
            MaxDepthInOpSubRegions = DepthInThisSubRegion;
          }
        }
        CurrentOpNestingLevel += MaxDepthInOpSubRegions;
      }

      if (CurrentOpNestingLevel > MaxDepthFound) {
        MaxDepthFound = CurrentOpNestingLevel;
      }
    }
  }
  return MaxDepthFound;
}

class MyLoopDepthPass
    : public PassWrapper<MyLoopDepthPass, OperationPass<func::FuncOp>> {
public:
  StringRef getArgument() const final { return "my-loop-depth-pass"; }

  StringRef getDescription() const final {
    return "Считает максимальную глубину вложенности регионов в циклах.";
  }

  void runOnOperation() override {
    func::FuncOp CurrentFunction = getOperation();
    OpBuilder Builder(CurrentFunction.getContext());

    SmallVector<int64_t, 4> DepthsOfAllLoopsInFunction;

    CurrentFunction.walk([&](Operation *OpInFunction) {
      if (isa<scf::ForOp, scf::WhileOp, affine::AffineForOp>(OpInFunction)) {
        int TotalDepthForThisLoop = 1;
        int MaxNestingInsideLoopBody = 0;

        for (Region &LoopRegion : OpInFunction->getRegions()) {
          int DepthInThisRegionOfLoop = calculateDepthInsideRegion(LoopRegion);
          if (DepthInThisRegionOfLoop > MaxNestingInsideLoopBody) {
            MaxNestingInsideLoopBody = DepthInThisRegionOfLoop;
          }
        }
        TotalDepthForThisLoop += MaxNestingInsideLoopBody;
        DepthsOfAllLoopsInFunction.push_back(TotalDepthForThisLoop);
      }
    });

    if (!DepthsOfAllLoopsInFunction.empty()) {
      ArrayAttr DepthsAttribute =
          Builder.getI64ArrayAttr(DepthsOfAllLoopsInFunction);
      CurrentFunction->setAttr("my_loop_depths", DepthsAttribute);

      llvm::outs() << "Функция '" << CurrentFunction.getName()
                   << "': обработана. Найденные глубины циклов: ";
      llvm::interleaveComma(DepthsOfAllLoopsInFunction, llvm::outs());
      llvm::outs() << "\n";
    } else {
      llvm::outs() << "Функция '" << CurrentFunction.getName()
                   << "': не содержит отслеживаемых циклов\n";
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MyLoopDepthPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MyLoopDepthPass)

static mlir::PassPluginLibraryInfo getMyLoopDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MyLoopDepthPlugin", "1.0",
          []() { mlir::PassRegistration<MyLoopDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMyLoopDepthPassPluginInfo();
}