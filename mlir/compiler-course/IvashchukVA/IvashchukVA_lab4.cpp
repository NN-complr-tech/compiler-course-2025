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
    ModuleOp Module = getOperation();

    // Считаем вызовы для каждой функции
    llvm::StringMap<int> CallCounts;

    Module.walk([&](func::CallOp CallOp) {
      StringRef Callee = CallOp.getCallee();
      CallCounts[Callee]++;
    });

    // Добавляем атрибуты к операциям вызова
    Module.walk([&](func::CallOp CallOp) {
      StringRef Callee = CallOp.getCallee();
      auto Count = CallCounts[Callee];
      if (Count > 0) {
        CallOp->setAttr(
            "call_count",
            IntegerAttr::get(IntegerType::get(&getContext(), 64), Count));
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CallCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CallCounterPass)

static mlir::PassPluginLibraryInfo getCallCounterPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CallCounterPass", "1.0",
          []() { PassRegistration<CallCounterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getCallCounterPluginInfo();
}