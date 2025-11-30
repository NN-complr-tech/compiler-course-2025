#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
struct CallCounterPass
    : public PassWrapper<CallCounterPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(CallCounterPass)

  StringRef getArgument() const final { return "call-counter"; }
  StringRef getDescription() const final { return "Count function calls"; }

  void runOnOperation() override {
    ModuleOp module = getOperation();

    // Считаем вызовы для каждой функции
    llvm::StringMap<int> callCounts;

    module.walk([&](func::CallOp callOp) {
      StringRef callee = callOp.getCallee();
      callCounts[callee]++;
    });

    // Добавляем атрибуты к операциям вызова
    module.walk([&](func::CallOp callOp) {
      StringRef callee = callOp.getCallee();
      auto count = callCounts[callee];
      if (count > 0) {
        callOp->setAttr(
            "call_count",
            IntegerAttr::get(IntegerType::get(&getContext(), 64), count));
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CallCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CallCounterPass)

mlir::PassPluginLibraryInfo getCallCounterPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CallCounterPass", "1.0",
          []() { PassRegistration<CallCounterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getCallCounterPluginInfo();
}