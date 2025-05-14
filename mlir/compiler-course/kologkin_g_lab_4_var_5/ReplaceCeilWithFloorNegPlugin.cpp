#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {

void replaceCeilWithNegFloor(ModuleOp module) {

  for (auto func : module.getOps<mlir::func::FuncOp>()) {

    func.walk([&](Operation *op) {
      if (auto ceilOp = dyn_cast<math::CeilOp>(op)) {
        Location loc = ceilOp->getLoc();
        Value input = ceilOp->getOperand(0);
        auto type = input.getType();

        if (auto vectorType = type.dyn_cast<VectorType>()) {
          if (!vectorType.getElementType().isa<FloatType>())
            return;
        } else if (!type.isa<FloatType>()) {
          return;
        }

        OpBuilder builder(ceilOp);
        builder.setInsertionPoint(ceilOp);

        auto neg1 = builder.create<arith::NegFOp>(loc, type, input);

        auto floorNeg = builder.create<math::FloorOp>(loc, type, neg1);

        auto neg2 = builder.create<arith::NegFOp>(loc, type, floorNeg);

        ceilOp->replaceAllUsesWith(neg2);

        ceilOp->erase();
      }
    });
  }
}

class ReplaceCeilWithFloorNegPlugin
    : public PassWrapper<ReplaceCeilWithFloorNegPlugin,
                         OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "ReplaceCeilWithFloorNegPlugin_KolodkinGrigorii_FIIT3_MLIR";
  }
  StringRef getDescription() const final { return "Description pass"; }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    replaceCeilWithNegFloor(module);
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ReplaceCeilWithFloorNegPlugin)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ReplaceCeilWithFloorNegPlugin)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ReplaceCeilWithFloorNegPlugin", "1.0",
          []() { PassRegistration<ReplaceCeilWithFloorNegPlugin>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
