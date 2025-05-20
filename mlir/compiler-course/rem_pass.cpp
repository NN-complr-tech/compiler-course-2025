#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

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
  void expandRemainder(Operation *op, OpBuilder &bld) {
    Value lhsVal = op->getOperand(0);
    Value rhsVal = op->getOperand(1);
    Location loc = op->getLoc();

    bld.setInsertionPoint(op);

    if (auto rhsConst =
            dyn_cast_or_null<arith::ConstantIntOp>(rhsVal.getDefiningOp())) {
      if (rhsConst.value() == 0) {
        op->emitError("division by zero");
        return signalPassFailure();
      }
    }

    Value divVal = bld.create<DivOp>(loc, lhsVal, rhsVal);
    Value mulVal = bld.create<arith::MulIOp>(loc, divVal, rhsVal);
    Value subVal = bld.create<arith::SubIOp>(loc, lhsVal, mulVal);

    op->getResult(0).replaceAllUsesWith(subVal);
    op->erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass)

static mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemPass", "1.0",
          []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
