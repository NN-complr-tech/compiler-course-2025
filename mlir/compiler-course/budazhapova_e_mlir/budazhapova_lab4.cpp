#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {

class LoopIterationMarker
    : public PassWrapper<LoopIterationMarker, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "loop-iteration-marker"; }

  StringRef getDescription() const final {
    return "Annotate loops with static iteration count information";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    Builder builder(module.getContext());

    module.walk([&](scf::ForOp loopOp) {
      if (loopOp->hasAttr("trip_count"))
        return;

      auto lb = getConstantValue(loopOp.getLowerBound());
      auto ub = getConstantValue(loopOp.getUpperBound());
      auto st = getConstantValue(loopOp.getStep());

      if (!lb || !ub || !st)
        return;

      int64_t start = lb.value();
      int64_t end = ub.value();
      int64_t step = st.value();

      if (step == 0)
        return;

      int64_t trips = 0;
      if (step > 0 && start < end) {
        trips = (end - start + step - 1) / step;
      } else if (step < 0 && start > end) {
        trips = (start - end - step - 1) / (-step);
      }

      if (trips >= 0) {
        loopOp->setAttr("trip_count", builder.getI64IntegerAttr(trips));
      }
    });
  }

private:
  std::optional<int64_t> getConstantValue(Value v) {
    if (auto c = v.getDefiningOp<arith::ConstantIndexOp>()) {
      return c.value();
    }
    if (auto c = v.getDefiningOp<arith::ConstantIntOp>()) {
      return c.value();
    }
    return std::nullopt;
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopIterationMarker)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopIterationMarker)

mlir::PassPluginLibraryInfo getLoopIterationMarkerPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopIterationMarker", "1.0",
          []() { PassRegistration<LoopIterationMarker>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLoopIterationMarkerPluginInfo();
}
