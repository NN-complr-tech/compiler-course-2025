#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class IterCounterPass
    : public PassWrapper<IterCounterPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "IterCounterPass_Ivanov_Mikhail_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "Annotate scf.for loops with a 'trip_count' attribute if the number "
           "of "
           "iterations is statically known.";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<scf::SCFDialect, arith::ArithDialect>();
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    auto *ctx = module.getContext();

    module.walk([&](scf::ForOp forOp) {
      if (forOp->hasAttr("trip_count")) {
        return;
      }

      auto lowerBound = forOp.getLowerBound();
      auto upperBound = forOp.getUpperBound();
      auto step = forOp.getStep();

      auto lbCstIdx = lowerBound.getDefiningOp<arith::ConstantIndexOp>();
      auto ubCstIdx = upperBound.getDefiningOp<arith::ConstantIndexOp>();
      auto stCstIdx = step.getDefiningOp<arith::ConstantIndexOp>();

      if (!lbCstIdx || !ubCstIdx || !stCstIdx || stCstIdx.value() == 0) {
        return;
      }

      int64_t lb = lbCstIdx.value();
      int64_t ub = ubCstIdx.value();
      int64_t st = stCstIdx.value();

      int64_t tripCount = 0;
      if (st > 0 && lb < ub) {
        tripCount = (ub - lb + st - 1) / st;
      } else if (st < 0 && lb > ub) {
        int64_t stAbs = -st;
        tripCount = (lb - ub + stAbs - 1) / stAbs;
      }

      forOp->setAttr("trip_count",
                     IntegerAttr::get(IndexType::get(ctx), tripCount));
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(IterCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(IterCounterPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "IterCounterPass", "1.0",
          []() { mlir::PassRegistration<IterCounterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
