#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

static std::optional<int64_t> getConstantIndex(Value v) {
  if (auto cst = v.getDefiningOp<arith::ConstantOp>()) {
    if (auto intAttr = mlir::dyn_cast<IntegerAttr>(cst.getValue()))
      return intAttr.getValue().getSExtValue();
  }
  return std::nullopt;
}

// Pass: scf.for -> "trip_count".
struct TripCountPass
    : public PassWrapper<TripCountPass, OperationPass<ModuleOp>> {

  StringRef getArgument() const final {
    return "TripCountPass_Lavrentyev_Alexey_FIIT3_MLIR";
  }
  StringRef getDescription() const final {
    return "Annotate scf.for with a \"trip_count\" attribute when the number "
           "of iterations can be determined at compile time.";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<scf::SCFDialect, arith::ArithDialect>();
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module.getContext());

    module.walk([&](scf::ForOp forOp) {
      // skip if already exists
      if (forOp->hasAttr("trip_count"))
        return;

      auto lbOpt = getConstantIndex(forOp.getLowerBound());
      auto ubOpt = getConstantIndex(forOp.getUpperBound());
      auto stepOpt = getConstantIndex(forOp.getStep());

      if (!lbOpt || !ubOpt || !stepOpt || *stepOpt == 0)
        return; // no attribute if parameters are wrong

      int64_t lb = *lbOpt;
      int64_t ub = *ubOpt;
      int64_t step = *stepOpt;

      // both + and -
      int64_t stepAbs = step > 0 ? step : -step;
      int64_t diff = step > 0 ? (ub - lb) : (lb - ub);
      int64_t tripCnt = diff <= 0 ? 0 : (diff + stepAbs - 1) / stepAbs;

      builder.setInsertionPoint(forOp);
      forOp->setAttr("trip_count", builder.getI64IntegerAttr(tripCnt));
    });
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TripCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TripCountPass)

mlir::PassPluginLibraryInfo getAnnotateTripCountPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TripCountPass", "1.0",
          []() { mlir::PassRegistration<TripCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getAnnotateTripCountPassPluginInfo();
}
