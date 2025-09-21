#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

static std::optional<int64_t> constIndex(Value v) {
  if (auto c = v.getDefiningOp<arith::ConstantIndexOp>())
    return static_cast<int64_t>(c.value());
  return std::nullopt;
}

static int64_t ceilDivAbs(int64_t a, int64_t b) {
  a = a < 0 ? -a : a;
  b = b < 0 ? -b : b;
  return (a + b - 1) / b;
}

static bool sameDirection(int64_t delta, int64_t step) {
  if (step == 0 || delta == 0)
    return false;
  return (delta > 0 && step > 0) || (delta < 0 && step < 0);
}

class AnnotateTripCountPass
    : public PassWrapper<AnnotateTripCountPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "AnnotateTripCount_Bessonov_Egor_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Attach a `trip_count` attribute to scf.for when the iteration "
           "count is statically computable.";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module.getContext());

    module.walk([&](scf::ForOp forOp) {
      if (forOp->hasAttr("trip_count"))
        return;

      auto lb = constIndex(forOp.getLowerBound());
      auto ub = constIndex(forOp.getUpperBound());
      auto st = constIndex(forOp.getStep());

      if (!lb || !ub || !st)
        return;

      int64_t delta = *ub - *lb;
      int64_t step = *st;

      if (!sameDirection(delta, step))
        return;

      int64_t trips = ceilDivAbs(delta, step);
      if (trips <= 0)
        return;

      builder.setInsertionPoint(forOp);
      forOp->setAttr("trip_count", builder.getI64IntegerAttr(trips));
    });
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(AnnotateTripCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(AnnotateTripCountPass)

mlir::PassPluginLibraryInfo getAnnotateTripCountPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "AnnotateTripCount", "1.0",
          []() { PassRegistration<AnnotateTripCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getAnnotateTripCountPluginInfo();
}
