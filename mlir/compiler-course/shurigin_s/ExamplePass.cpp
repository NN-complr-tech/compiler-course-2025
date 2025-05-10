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

  for (Block &block : InputRegion) {
    for (Operation &op : block.getOperations()) {

      int currentOpNestingLevel = 0;

      bool isNestableStructure =
          isa<scf::ForOp, scf::WhileOp, affine::AffineForOp, scf::IfOp,
              affine::AffineIfOp>(&op);

      if (isNestableStructure) {
        currentOpNestingLevel = 1;

        int maxDepthInSubRegions = 0;
        for (Region &subRegion : op.getRegions()) {
          int depthInThisSubRegion = calculateDepthInsideRegion(subRegion);
          if (depthInThisSubRegion > maxDepthInSubRegions) {
            maxDepthInSubRegions = depthInThisSubRegion;
          }
        }
        currentOpNestingLevel += maxDepthInSubRegions;
      }

      if (currentOpNestingLevel > maxDepthFound) {
        MaxDepthFound = currentOpNestingLevel;
      }
    }
  }
  return maxDepthFound;
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
      bool IsLoopWeCareAbout = false;
      if (isa<scf::ForOp>(OpInFunction) || isa<scf::WhileOp>(OpInFunction) ||
          isa<affine::AffineForOp>(OpInFunction)) {
        IsLoopWeCareAbout = true;
      }

      if (IsLoopWeCareAbout) {
        int TotalDepthForThisLoop = 1;

        int MaxNestingInsideLoopBody = 0;

        for (Region &loopRegion : OpInFunction->getRegions()) {
          int depthInThisRegionOfLoop = calculateDepthInsideRegion(loopRegion);

          if (depthInThisRegionOfLoop > maxNestingInsideLoopBody) {
            MaxNestingInsideLoopBody = depthInThisRegionOfLoop;
          }
        }

        TotalDepthForThisLoop += maxNestingInsideLoopBody;

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
                   << "': не содержит отслеживаемых циклов (scf.for, "
                      "scf.while, affine.for).\n";
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MyLoopDepthPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MyLoopDepthPass)

mlir::PassPluginLibraryInfo getMyLoopDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MyLoopDepthPlugin", "1.0",
          []() { mlir::PassRegistration<MyLoopDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMyLoopDepthPassPluginInfo();
}