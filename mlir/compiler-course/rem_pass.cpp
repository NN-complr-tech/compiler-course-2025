#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

#define PLUGIN_NAME "rem_pass_Mamaeva_Olga_FIIT3_MLIR"

using namespace mlir;

namespace {
class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
private:
  void expandRemainder(Operation *op, OpBuilder &builder, bool isSigned) {
    Value lhs = op->getOperand(0);
    Value rhs = op->getOperand(1);
    Location loc = op->getLoc();

    Value div = isSigned
                    ? builder.create<arith::DivSIOp>(loc, lhs, rhs).getResult()
                    : builder.create<arith::DivUIOp>(loc, lhs, rhs).getResult();
    Value mul = builder.create<arith::MulIOp>(loc, div, rhs).getResult();
    Value result = builder.create<arith::SubIOp>(loc, lhs, mul).getResult();

    op->replaceAllUsesWith(result);
    op->erase();
  }

public:
  StringRef getArgument() const final { return PLUGIN_NAME; }

  StringRef getDescription() const final {
    return "Replace remainder ops with div+mul+sub sequence";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    module.walk([&](arith::RemSIOp op) {
      expandRemainder(op, builder, /*isSigned=*/true);
    });

    module.walk([&](arith::RemUIOp op) {
      expandRemainder(op, builder, /*isSigned=*/false);
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

namespace {
mlir::PassPluginLibraryInfo getMamaevaPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, PLUGIN_NAME, "1.0",
          []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaPluginInfo();
}
