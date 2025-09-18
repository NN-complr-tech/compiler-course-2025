#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class myMaxRegionNestingAnalyzer
    : public PassWrapper<myMaxRegionNestingAnalyzer, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "SearchMaxDepthPass_Lysov_Ivan_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Compute the maximum nesting depth of loop-related region "
           "structures inside func.func";
  }

  unsigned measureRegionNesting(Operation *startingOp,
                                unsigned currentLevel = 1) {
    unsigned deepestLevel = currentLevel;

    for (Region &region : startingOp->getRegions()) {
      for (Block &block : region) {
        for (Operation &op : block) {
          if (isa<affine::AffineForOp, affine::AffineIfOp, scf::ForOp,
                  scf::IfOp, scf::WhileOp>(&op)) {
            unsigned nestedLevel = measureRegionNesting(&op, currentLevel + 1);
            deepestLevel = std::max(deepestLevel, nestedLevel);
          }
        }
      }
    }

    return deepestLevel;
  }

  void runOnOperation() override {
    ModuleOp rootModule = getOperation();
    OpBuilder irBuilder(rootModule.getContext());

    rootModule.walk([&](func::FuncOp func) {
      unsigned maximumDepth = 0;

      func.walk([&](Operation *candidateLoop) {
        if (isa<scf::ForOp, scf::WhileOp, affine::AffineForOp>(candidateLoop)) {
          unsigned depthHere = measureRegionNesting(candidateLoop);
          maximumDepth = std::max(maximumDepth, depthHere);
        }
      });

      func->setAttr("loop_max_depth",
                    irBuilder.getI32IntegerAttr(maximumDepth));
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(myMaxRegionNestingAnalyzer)
MLIR_DEFINE_EXPLICIT_TYPE_ID(myMaxRegionNestingAnalyzer)

mlir::PassPluginLibraryInfo getLoopDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "SearchMaxDepthPass_Lysov_Ivan_FIIT3_MLIR",
          "1.0", []() { PassRegistration<myMaxRegionNestingAnalyzer>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLoopDepthPassPluginInfo();
}
