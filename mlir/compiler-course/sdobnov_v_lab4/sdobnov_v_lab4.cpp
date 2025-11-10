#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class CallFrequencyAnalyzer
    : public PassWrapper<CallFrequencyAnalyzer, OperationPass<ModuleOp>> {
private:
  void analyzeCallPatterns(ModuleOp module,
                           llvm::StringMap<int> &frequencyData) {
    module.walk([&](func::CallOp callOp) {
      StringRef calleeName = callOp.getCallee();
      frequencyData[calleeName]++;
    });
  }

  void annotateCallOperations(ModuleOp module,
                              const llvm::StringMap<int> &frequencyData) {
    OpBuilder builder(module.getContext());
    module.walk([&](func::CallOp callOp) {
      StringRef calleeName = callOp.getCallee();
      auto callCount = frequencyData.lookup(calleeName);
      IntegerAttr countAttr = builder.getI64IntegerAttr(callCount);
      callOp->setAttr("invocation_count", countAttr);
    });
  }

public:
  void runOnOperation() override {
    ModuleOp module = getOperation();
    llvm::StringMap<int> callStatistics;

    analyzeCallPatterns(module, callStatistics);
    annotateCallOperations(module, callStatistics);
  }

  StringRef getArgument() const final { return "call-frequency-analyzer"; }

  StringRef getDescription() const final {
    return "Analyzes function call frequency and annotates call operations "
           "with invocation counts";
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CallFrequencyAnalyzer)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CallFrequencyAnalyzer)

mlir::PassPluginLibraryInfo getCallAnalyzerPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CallFrequencyAnalyzer", "1.0",
          []() { PassRegistration<CallFrequencyAnalyzer>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getCallAnalyzerPluginInfo();
}