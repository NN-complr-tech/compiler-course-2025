#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class FunctionCallTrackerPass
    : public PassWrapper<FunctionCallTrackerPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "FunctionCallTracker_Korobeinikov_Arseny_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "Counts total number of calls to each function in the module";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module.getContext());

    llvm::StringMap<int64_t> globalCallCounts;
    llvm::SmallVector<func::CallOp> callOpsCache;

    // Single walk to collect data and verify functions
    module.walk([&](func::CallOp callOp) {
      StringRef callee = callOp.getCallee();

      globalCallCounts[callee]++;
      callOpsCache.push_back(callOp); // Change 1: Cache call operations
    });

    // Annotate cached calls
    for (func::CallOp callOp : callOpsCache) {
      StringRef callee = callOp.getCallee();
      int64_t totalCalls = globalCallCounts.lookup(callee);
      callOp->setAttr(
          "invoke_total",
          builder.getI64IntegerAttr(totalCalls)); // Changed attribute name
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(FunctionCallTrackerPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(FunctionCallTrackerPass)

mlir::PassPluginLibraryInfo getFunctionCallTrackerPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "FunctionCallTracker", "1.0",
          []() { mlir::PassRegistration<FunctionCallTrackerPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallTrackerPluginInfo();
}