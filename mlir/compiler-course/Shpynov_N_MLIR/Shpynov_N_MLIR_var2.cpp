#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

struct AnnotatorPass
    : public PassWrapper<AnnotatorPass, OperationPass<ModuleOp>> {
  StringRef getArgument() const final {
    return "AnnotatorPass_Shpynov_Nikita_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "Annotate scf.for loops with an attribute \"trip_count\" that "
           "represents amount of iteration. If trip_count is unknown then no "
           "attribute is attached";
  }
  void runOnOperation() override {
    ModuleOp module = getOperation();

    module.walk([&](scf::ForOp forOp) {
      // get the step, lower bound, and upper bound of the loop
      // as constant index operations. If any are not constants, we cannot
      // compute the trip count.
      auto stepConst =
          forOp.getStep().getDefiningOp<arith::ConstantIndexOp>(); //
      auto lowerConst =
          forOp.getLowerBound().getDefiningOp<arith::ConstantIndexOp>();
      auto upperConst =
          forOp.getUpperBound().getDefiningOp<arith::ConstantIndexOp>();
// If any of the bounds or step are unknown, skip this loop
      if (!stepConst || !lowerConst || !upperConst)
        return;

      if (stepConst && lowerConst && upperConst) {
        int64_t step = stepConst.value();
        if (step == 0) // Skip loops with zero step
          return;
        int64_t lower = lowerConst.value();
        int64_t upper = upperConst.value();
        int64_t tripCount = (upper - lower + step - 1) / step;

        // Compute trip count depending on the step direction
        if (step > 0 && lower < upper) {
            // Positive step, from lower to upper
          tripCount = (upper - lower + step - 1) / step;

        } else if (step < 0 && lower > upper) {
            // Negative step, from upper to lower
          tripCount = (lower - upper - step - 1) / (-step);
        }
        // Attach the attribute if trip count is non-negative
        if (tripCount >= 0) {
          forOp->setAttr(
              "trip_count",
              IntegerAttr::get(IntegerType::get(forOp.getContext(), 64),
                               tripCount));
        }
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(AnnotatorPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(AnnotatorPass)

mlir::PassPluginLibraryInfo getAnnotatorPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "AnnotatorPass", "1.0",
          []() { mlir::PassRegistration<AnnotatorPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getAnnotatorPassPluginInfo();
}
