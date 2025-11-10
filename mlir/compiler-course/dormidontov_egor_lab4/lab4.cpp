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
int computeMaxLoopDepth(func::FuncOp funcOp) {
  int maxDepth = 0;
  std::function<void(Operation *, int)> visit = [&](Operation *op,
                                                    int curDepth) {
    if (isa<scf::ForOp, scf::WhileOp, scf::IfOp, affine::AffineForOp>(op))
      ++curDepth;
    if (curDepth > maxDepth)
      maxDepth = curDepth;
    for (Region &region : op->getRegions()) {
      for (Block &block : region) {
        for (Operation &nestedOp : block) {
          visit(&nestedOp, curDepth);
        }
      }
    }
  };

  visit(funcOp.getOperation(), 0);

  return maxDepth;
}

class MaxLoopRegionDepthPass
    : public PassWrapper<MaxLoopRegionDepthPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "MaxLoopRegionDepthPass_Dormidontov_Egor_FIIT2_MLIR";
  }
  StringRef getDescription() const final {
    return "Attach max region nest depth for each loop to function";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    moduleOp.walk([&](func::FuncOp funcOp) {
      int maxDepth = computeMaxLoopDepth(funcOp);
      funcOp->setAttr("max_loop_region_depth",
                      builder.getI32IntegerAttr(maxDepth));
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MaxLoopRegionDepthPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MaxLoopRegionDepthPass)

mlir::PassPluginLibraryInfo getMaxLoopRegionDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MaxLoopRegionDepthPass", "1.0",
          []() { mlir::PassRegistration<MaxLoopRegionDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMaxLoopRegionDepthPassPluginInfo();
}
