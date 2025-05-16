#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class ExpandRemsiPass
    : public PassWrapper<ExpandRemsiPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "ExpandRemsiPass_Ionova_Ekaterina_FIIT1_MLIR";
  }
  StringRef getDescription() const final { return "Description pass"; }

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
    auto lhs = op->getOperand(0);
    auto rhs = op->getOperand(1);
    auto loc = op->getLoc();

    builder.setInsertionPoint(op);

    auto div = builder.create<DivOp>(loc, lhs, rhs);
    auto mul = builder.create<arith::MulIOp>(loc, div, rhs);
    auto sub = builder.create<arith::SubIOp>(loc, lhs, mul);

    op->replaceAllUsesWith(sub);
    op->erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ExpandRemsiPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ExpandRemsiPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ExpandRemsiPass", "1.0",
          []() { mlir::PassRegistration<ExpandRemsiPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
