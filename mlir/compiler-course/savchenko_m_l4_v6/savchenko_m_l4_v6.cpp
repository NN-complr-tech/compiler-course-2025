#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
struct FunctionCallAnalyzerPass
    : public PassWrapper<FunctionCallAnalyzerPass, OperationPass<ModuleOp>> {

  StringRef getArgument() const final {
    return "FunctionCallAnalyzer_Savchenko_Maxim_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "Annotates each func.call op with total number of times the target "
           "function is called in the module.";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    auto *context = module.getContext();
    llvm::DenseMap<StringRef, int64_t> callFrequencyMap;

    for (auto funcOp : module.getOps<func::FuncOp>()) {
      auto uses = SymbolTable::getSymbolUses(funcOp, module);
      if (!uses)
        continue;

      int64_t count = 0;
      for (const auto &use : *uses) {
        if (isa<func::CallOp>(use.getUser())) {
          count++;
        }
      }

      callFrequencyMap[funcOp.getSymName()] = count;
    }

    OpBuilder builder(context);

    // Second walk — adding attributes
    module.walk([&](func::CallOp callOp) {
      auto calleeName = callOp.getCallee();
      int64_t callCount = callFrequencyMap.lookup(calleeName);
      auto attr = builder.getI64IntegerAttr(callCount);
      callOp->setAttr("call_count", attr);
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(FunctionCallAnalyzerPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(FunctionCallAnalyzerPass)

static PassPluginLibraryInfo getFunctionCallAnalyzerPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "FunctionCallAnalyzer_Savchenko_Maxim_FIIT1_MLIR", "1.0",
          []() { PassRegistration<FunctionCallAnalyzerPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getFunctionCallAnalyzerPassPluginInfo();
}