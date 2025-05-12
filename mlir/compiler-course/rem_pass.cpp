#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
private:
  void expandRemainder(Operation *op, OpBuilder &Builder, bool IsSigned) {
    Value Lhs = op->getOperand(0);
    Value Rhs = op->getOperand(1);
    Location Loc = op->getLoc();

    Value Div;
    if (IsSigned) {
      Div = Builder.create<arith::DivSIOp>(Loc, Lhs, Rhs).getResult();
    } else {
      Div = Builder.create<arith::DivUIOp>(Loc, Lhs, Rhs).getResult();
    }

    Value Mul = Builder.create<arith::MulIOp>(Loc, Div, Rhs).getResult();
    Value Result = Builder.create<arith::SubIOp>(Loc, Lhs, Mul).getResult();

    op->getResult(0).replaceAllUsesWith(Result);
    op->erase();
  }

public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Replace remainder ops with div+mul+sub sequence";
  }

  void runOnOperation() override {
    ModuleOp Module = getOperation();
    OpBuilder Builder(Module);

    Module.walk([&](arith::RemSIOp Op) {
      expandRemainder(Op, Builder, /*IsSigned=*/true);
    });

    Module.walk([&](arith::RemUIOp Op) {
      expandRemainder(Op, Builder, /*IsSigned=*/false);
    });
  }
};

static mlir::PassPluginLibraryInfo getMamaevaRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3", "1.0",
          []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaRemPassPluginInfo();
}
