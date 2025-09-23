#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include <cmath>

using namespace mlir;

namespace {

class ForLoopTripCountAnnotator
    : public PassWrapper<ForLoopTripCountAnnotator, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "ForLoopTripCountAnnotator_Mezhuev_Maksim_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Annotates SCF for loops with trip_count attribute indicating "
           "iteration count";
  }

  bool areLoopBoundsValid(int64_t lower, int64_t upper, int64_t step) {
    if (step == 0)
      return false;
    if (step > 0)
      return upper > lower;
    return upper < lower;
  }

  int64_t calculateTripCount(int64_t lower, int64_t upper, int64_t step) {
    int64_t distance = std::abs(upper - lower);
    int64_t absoluteStep = std::abs(step);
    return (distance + absoluteStep - 1) / absoluteStep;
  }

  bool extractConstantValue(Value value, int64_t &result) {
    if (auto constantOp =
            dyn_cast_or_null<arith::ConstantIndexOp>(value.getDefiningOp())) {
      result = constantOp.value();
      return true;
    }
    return false;
  }

  void processSCFForLoop(scf::ForOp forOp) {
    int64_t lowerValue, upperValue, stepValue;

    bool hasLower = extractConstantValue(forOp.getLowerBound(), lowerValue);
    bool hasUpper = extractConstantValue(forOp.getUpperBound(), upperValue);
    bool hasStep = extractConstantValue(forOp.getStep(), stepValue);

    if (hasLower && hasUpper && hasStep) {
      if (areLoopBoundsValid(lowerValue, upperValue, stepValue)) {
        int64_t iterationCount =
            calculateTripCount(lowerValue, upperValue, stepValue);
        OpBuilder builder(forOp);
        forOp->setAttr("trip_count", builder.getI64IntegerAttr(iterationCount));
      }
    }
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    module.walk([this](scf::ForOp forOp) { processSCFForLoop(forOp); });
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ForLoopTripCountAnnotator)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ForLoopTripCountAnnotator)

mlir::PassPluginLibraryInfo getTripCountAnnotatorPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ForLoopTripCountAnnotator", "1.0",
          []() { PassRegistration<ForLoopTripCountAnnotator>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTripCountAnnotatorPluginInfo();
}