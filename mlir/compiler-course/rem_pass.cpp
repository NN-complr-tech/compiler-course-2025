#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Replace remainder ops with equivalent arithmetic operations";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    moduleOp.walk([&](arith::RemSIOp op) {
      expandRemainder(op, arith::DivSIOp(), builder);
    });

    moduleOp.walk([&](arith::RemUIOp op) {
      expandRemainder(op, arith::DivUIOp(), builder);
    });
  }

private:
  template <typename DivOp>
  void expandRemainder(Operation *op, DivOp divOp, OpBuilder &builder) {
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

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

mlir::PassPluginLibraryInfo getMamaevaRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3", "1.0",
          []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaRemPassPluginInfo();
}
