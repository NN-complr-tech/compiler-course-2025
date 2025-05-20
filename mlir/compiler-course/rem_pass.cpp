#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class RemPass : public PassWrapper<RemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "rem-pass"; }

  StringRef getDescription() const final {
    return "Replace remainder operations with div+mul+sub sequence";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    module.walk([&](arith::RemSIOp remOp) {
      expandRemainder<arith::DivSIOp>(remOp, builder);
    });

    module.walk([&](arith::RemUIOp remOp) {
      expandRemainder<arith::DivUIOp>(remOp, builder);
    });
  }

private:
  template <typename DivOp>
  void expandRemainder(Operation *op, OpBuilder &builder) {
    Value lhs = op->getOperand(0);
    Value rhs = op->getOperand(1);
    Location loc = op->getLoc();

    builder.setInsertionPoint(op);

    if (auto rhsConst =
            dyn_cast_or_null<arith::ConstantIntOp>(rhs.getDefiningOp())) {
      if (rhsConst.value() == 0) {
        op->emitError("division by zero");
        return signalPassFailure();
      }
    }

    Value div = builder.create<DivOp>(loc, lhs, rhs);
    Value mul = builder.create<arith::MulIOp>(loc, div, rhs);
    Value sub = builder.create<arith::SubIOp>(loc, lhs, mul);

    op->getResult(0).replaceAllUsesWith(sub);
    op->erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass)

static mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "RemPass", // Имя плагина
          "1.0", []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
