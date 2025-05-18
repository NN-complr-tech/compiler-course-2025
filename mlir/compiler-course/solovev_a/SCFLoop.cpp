#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class ScfForLoopsPass
    : public PassWrapper<ScfForLoopsPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "Lab_4_mlir_Solovev_a_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "Annotate scf.for loops with trip_count attribute if known";
  }
  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    moduleOp.walk([&](scf::ForOp forOp) {
      auto lowerBound =
          forOp.getLowerBound().getDefiningOp<arith::ConstantIndexOp>();
      auto upperBound =
          forOp.getUpperBound().getDefiningOp<arith::ConstantIndexOp>();
      auto step = forOp.getStep().getDefiningOp<arith::ConstantIndexOp>();
      if (lowerBound && upperBound && step) {
        int64_t lb = lowerBound.value();
        int64_t ub = upperBound.value();
        int64_t st = step.value();
        if (st == 0)
          return;
        int64_t tripCount = 0;
        if ((st > 0 && lb < ub) || (st < 0 && lb > ub)) {
          tripCount = (std::abs(ub - lb) + std::abs(st) - 1) / std::abs(st);
          forOp->setAttr(
              "trip_count",
              IntegerAttr::get(IndexType::get(forOp.getContext()), tripCount));
        }
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ScfForLoopsPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ScfForLoopsPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ScfForLoopsPass", "1.0",
          []() { mlir::PassRegistration<ScfForLoopsPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
