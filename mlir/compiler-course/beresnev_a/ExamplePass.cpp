#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class TripPass
    : public mlir::PassWrapper<TripPass, mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const final {
    return "UserPass_Beresnev_Anton_FIIT1_MLIR";
  }
  mlir::StringRef getDescription() const final {
    return "Annotate scf.for loops with an attribute trip_count";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

    module.walk([&](mlir::scf::ForOp forOp) {
      auto lowerAttr =
          forOp.getLowerBound().getDefiningOp<mlir::arith::ConstantIndexOp>();
      auto upperAttr =
          forOp.getUpperBound().getDefiningOp<mlir::arith::ConstantIndexOp>();
      auto stepAttr =
          forOp.getStep().getDefiningOp<mlir::arith::ConstantIndexOp>();

      if (!lowerAttr || !upperAttr || !stepAttr)
        return;

      int64_t lower = lowerAttr.value();
      int64_t upper = upperAttr.value();
      int64_t step = stepAttr.value();

      int64_t tripCount;
      if (step > 0) {
        tripCount = (upper - lower + step - 1) / step;
      } else if (step < 0) {
        tripCount = (-(upper - lower) - step - 1) / (-step);
      } else {
        if (upper - lower == 0) {
          tripCount = 0;
        } else
          return;
      }
      if (tripCount < 0)
        tripCount = 0;

      mlir::OpBuilder builder(forOp);
      auto attr = builder.getI64IntegerAttr(tripCount);
      forOp->setAttr("trip_count", attr);
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TripPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TripPass)

mlir::PassPluginLibraryInfo getTripPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TripPass", "1.0",
          []() { mlir::PassRegistration<TripPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTripPassPluginInfo();
}
