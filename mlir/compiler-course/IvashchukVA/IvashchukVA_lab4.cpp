#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace {
struct CallCounterPass
    : public mlir::PassWrapper<CallCounterPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

    module.walk([&](mlir::func::FuncOp func) {
      int callCount = 0;

      func.walk([&](mlir::Operation *op) {
        if (llvm::isa<mlir::func::CallOp>(op)) {
          callCount++;
        }
      });

      if (callCount > 0) {
        func->setAttr("call_count",
                      mlir::IntegerAttr::get(
                          mlir::IntegerType::get(func.getContext(), 64),
                          callCount));
      }
    });
  }
};
} // namespace

void registerCallCounterPass() {
  mlir::PassRegistration<CallCounterPass>(
      "call-counter", "Count function calls and add call_count attribute");
}