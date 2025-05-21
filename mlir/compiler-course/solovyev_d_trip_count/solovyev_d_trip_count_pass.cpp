#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
bool sameSign(int64_t a, int64_t b) {
  return (a > 0 && b > 0) || (a < 0 && b < 0);
}
class TripCountPass
    : public PassWrapper<TripCountPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "TripCountPass_Solovyev_Danila_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Annotate scf.for loops with an attribute \"trip_count\" that "
           "represents amount of iteration.";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);
    moduleOp.walk([&](scf::ForOp forOp) {
      auto lower = forOp.getLowerBound();
      auto upper = forOp.getUpperBound();
      auto step = forOp.getStep();

      auto lowerConst =
          dyn_cast_or_null<arith::ConstantIndexOp>(lower.getDefiningOp());
      auto upperConst =
          dyn_cast_or_null<arith::ConstantIndexOp>(upper.getDefiningOp());
      auto stepConst =
          dyn_cast_or_null<arith::ConstantIndexOp>(step.getDefiningOp());
      if (lowerConst && upperConst && stepConst && stepConst.value()) {
        if (sameSign((upperConst.value() - lowerConst.value()),
                     stepConst.value())) {
          int64_t tripCount =
              (std::abs(upperConst.value() - lowerConst.value()) +
               std::abs(stepConst.value()) - 1) /
              std::abs(stepConst.value());
          builder.setInsertionPoint(forOp);
          forOp->setAttr("trip_count", builder.getI64IntegerAttr(tripCount));
        }
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TripCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TripCountPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TripCountPass", "1.0",
          []() { PassRegistration<TripCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}