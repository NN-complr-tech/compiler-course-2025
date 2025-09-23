#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Utils/StaticValueUtils.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class TripCountPass
    : public PassWrapper<TripCountPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "TripCountPass_Shkurinskaya_Elena_FIIT2_MLIR";
  }
  StringRef getDescription() const final {
    return "Annotate scf.for loops with a trip_count";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *ctx = &getContext();
    OpBuilder builder(ctx);

    module.walk([&](scf::ForOp forOp) {
      if (forOp->hasAttr("trip_count"))
        return;

      auto lbOpt = getConstantIntValue(forOp.getLowerBound());
      auto ubOpt = getConstantIntValue(forOp.getUpperBound());
      auto stOpt = getConstantIntValue(forOp.getStep());
      if (!lbOpt || !ubOpt || !stOpt)
        return;

      int64_t lb = *lbOpt;
      int64_t ub = *ubOpt;
      int64_t st = *stOpt;
      if (st == 0)
        return;

      int64_t diff = ub - lb;
      int64_t count = 0;
      if (st > 0 && diff > 0) {
        count = (diff + st - 1) / st; // ceil(diff/st)
      } else if (st < 0 && diff < 0) {
        int64_t posStep = -st;
        count = ((-diff) + posStep - 1) / posStep;
      } else {
        return;
      }

      if (count <= 0)
        return;

      forOp->setAttr("trip_count", builder.getIndexAttr(count));
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TripCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TripCountPass)

mlir::PassPluginLibraryInfo getTripCountPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TripCountPass", "1.0",
          []() { PassRegistration<TripCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTripCountPassPluginInfo();
}
