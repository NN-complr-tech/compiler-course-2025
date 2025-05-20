#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class RemPass_Mamaeva_Olga_FIIT3_MLIR
    : public PassWrapper<RemPass_Mamaeva_Olga_FIIT3_MLIR,
                         OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "rem-pass-mamaeva-olga-fiit3-mlir"; // в kebab-case
  }

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

    Value div = builder.create<DivOp>(loc, lhs, rhs);
    Value mul = builder.create<arith::MulIOp>(loc, div, rhs);
    Value sub = builder.create<arith::SubIOp>(loc, lhs, mul);

    op->getResult(0).replaceAllUsesWith(sub);
    op->erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass_Mamaeva_Olga_FIIT3_MLIR)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass_Mamaeva_Olga_FIIT3_MLIR)

static mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "RemPass_Mamaeva_Olga_FIIT3_MLIR", // Должно совпадать с именем класса
          "1.0",
          []() { mlir::PassRegistration<RemPass_Mamaeva_Olga_FIIT3_MLIR>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
