#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/CallInterfaces.h"
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
    llvm::SmallVector<CallOpInterface> callOpsCache;

    module.walk([&](Operation *op) {
      if (auto call = dyn_cast<CallOpInterface>(op)) {
        if (auto calleeAttr =
                call.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
          StringRef calleeName = calleeAttr.getRootReference().getValue();
          globalCallCounts[calleeName]++;
          callOpsCache.push_back(call);
        }
      }
    });

    for (CallOpInterface call : callOpsCache) {
      if (auto calleeAttr =
              call.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
        StringRef calleeName = calleeAttr.getRootReference().getValue();
        int64_t totalCalls = globalCallCounts.lookup(calleeName);
        call->setAttr("invoke_total", builder.getI64IntegerAttr(totalCalls));
      }
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
