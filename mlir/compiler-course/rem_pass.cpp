#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR"; // Сохраняем ваше имя pass
  }

  StringRef getDescription() const final {
    return "Replace remainder ops with div+mul+sub sequence"; // Ваше описание
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    module.walk([&](arith::RemSIOp op) {
      expandRemainder<arith::DivSIOp>(op, builder);
    });

    module.walk([&](arith::RemUIOp op) {
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

    Value div = builder.create<DivOp>(loc, lhs, rhs);
    Value mul = builder.create<arith::MulIOp>(loc, div, rhs);
    Value result = builder.create<arith::SubIOp>(loc, lhs, mul);

    op->replaceAllUsesWith(result);
    op->erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

namespace {
mlir::PassPluginLibraryInfo getMamaevaRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "rem_pass_Mamaeva_Olga_FIIT3_MLIR", // Сохраняем ваше имя плагина
          "1.0", []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaRemPassPluginInfo();
}
