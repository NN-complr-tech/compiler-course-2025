#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

using namespace mlir;

namespace {
struct CallCounterPass
    : public PassWrapper<CallCounterPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  void runOnOperation() override {
    ModuleOp module = getOperation();

    for (auto func : module.getOps<func::FuncOp>()) {
      int count = 0;
      func.walk([&](Operation *op) {
        if (isa<func::CallOp>(op)) {
          count++;
        }
      });

      if (count > 0) {
        func->setAttr(
            "call_count",
            IntegerAttr::get(IntegerType::get(&getContext(), 64), count));
      }
    }
  }
};
} // namespace

void registerCallCounterPass() { PassRegistration<CallCounterPass>(); }