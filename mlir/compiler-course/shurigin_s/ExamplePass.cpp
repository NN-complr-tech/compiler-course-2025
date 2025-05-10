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

int calculateDepthInsideRegion(Region &inputRegion) {
  int maxDepthFound = 0;

  for (Block &block : inputRegion) {
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
        maxDepthFound = currentOpNestingLevel;
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
    func::FuncOp currentFunction = getOperation();
    OpBuilder builder(currentFunction.getContext());

    SmallVector<int64_t, 4> depthsOfAllLoopsInFunction;

    currentFunction.walk([&](Operation *opInFunction) {
      bool isLoopWeCareAbout = false;
      if (isa<scf::ForOp>(opInFunction) || isa<scf::WhileOp>(opInFunction) ||
          isa<affine::AffineForOp>(opInFunction)) {
        isLoopWeCareAbout = true;
      }

      if (isLoopWeCareAbout) {
        int totalDepthForThisLoop = 1;

        int maxNestingInsideLoopBody = 0;

        for (Region &loopRegion : opInFunction->getRegions()) {
          int depthInThisRegionOfLoop = calculateDepthInsideRegion(loopRegion);

          if (depthInThisRegionOfLoop > maxNestingInsideLoopBody) {
            maxNestingInsideLoopBody = depthInThisRegionOfLoop;
          }
        }

        totalDepthForThisLoop += maxNestingInsideLoopBody;

        depthsOfAllLoopsInFunction.push_back(totalDepthForThisLoop);
      }
    });

    if (!depthsOfAllLoopsInFunction.empty()) {
      ArrayAttr depthsAttribute =
          builder.getI64ArrayAttr(depthsOfAllLoopsInFunction);

      currentFunction->setAttr("my_loop_depths", depthsAttribute);

      llvm::outs() << "Функция '" << currentFunction.getName()
                   << "': обработана. Найденные глубины циклов: ";
      llvm::interleaveComma(depthsOfAllLoopsInFunction, llvm::outs());
      llvm::outs() << "\n";
    } else {
      llvm::outs() << "Функция '" << currentFunction.getName()
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