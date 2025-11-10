#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Operation.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class GenericCallCounter
    : public PassWrapper<GenericCallCounter, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "generic-call-counter-IvashchukVA";
  }

  StringRef getDescription() const final {
    return "Count function calls and add call_count attribute";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();

    SmallVector<CallOpInterface, 8> globalCalls;
    module.walk([&](CallOpInterface callOp) { globalCalls.push_back(callOp); });

    llvm::StringMap<int64_t> globalCounts;
    for (auto &call : globalCalls) {
      if (auto symCallAttr =
              call.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
        StringRef nameCallee = symCallAttr.getRootReference().getValue();
        globalCounts[nameCallee]++;
      }
    }

    OpBuilder builder(module.getContext());
    for (auto &call : globalCalls) {
      if (auto symCallAttr =
              call.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
        StringRef nameCallee = symCallAttr.getRootReference().getValue();
        int64_t n = globalCounts.lookup(nameCallee);
        call->setAttr("func.call_count", builder.getIndexAttr(n));
      }
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(GenericCallCounter)
MLIR_DEFINE_EXPLICIT_TYPE_ID(GenericCallCounter)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "GenericCallCounter", "1.2",
          []() { mlir::PassRegistration<GenericCallCounter>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}