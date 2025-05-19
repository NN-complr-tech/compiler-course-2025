#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class RemPass : public PassWrapper<RemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "RemPass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Replace remainder operations with div+mul+sub sequence";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    moduleOp.walk([&](arith::RemSIOp op) {
      expandRemainder<arith::DivSIOp>(op, builder);
    });

    moduleOp.walk([&](arith::RemUIOp op) {
      expandRemainder<arith::DivUIOp>(op, builder);
    });
  }

private:
  template <typename DivOp>
  void expandRemainder(Operation *op, OpBuilder &builder) {
    Value lhs = op->getOperand(0);
    Value rhs = op->getOperand(1);
    Location loc = op->getLoc();

    builder.setInsertionPoint(op);

    // Проверка деления на ноль для констант
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

mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemPass", "1.0",
          []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
