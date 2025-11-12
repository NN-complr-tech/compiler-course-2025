#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"

using namespace mlir;

namespace {
struct CallCounterPass
    : public PassWrapper<CallCounterPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  StringRef getArgument() const final {
    return "CallCounterPass_IvashchukVA_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Count function calls in MLIR";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();

    for (auto func : module.getOps<func::FuncOp>()) {
      int callCount = 0;

      func.walk([&](Operation *op) {
        if (isa<func::CallOp>(op)) {
          callCount++;
        }
      });

      if (callCount > 0) {
        func->setAttr(
            "call_count",
            IntegerAttr::get(IntegerType::get(module.getContext(), 64),
                             callCount));
      }
    }
  }
};
} // namespace

static mlir::PassRegistration<CallCounterPass> unused(
    "CallCounterPass_IvashchukVA_FIIT2_MLIR",
    "Count function calls in MLIR");
