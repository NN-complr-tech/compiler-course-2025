#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class FunctionCallCounterPass
    : public PassWrapper<FunctionCallCounterPass, OperationPass<ModuleOp>> {
public:
  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();

    llvm::StringMap<int> functionCallFrequencyMap;

    moduleOp.walk([&](func::CallOp callInstruction) {
      StringRef targetFunctionName = callInstruction.getCallee();
      functionCallFrequencyMap[targetFunctionName]++;
    });

    OpBuilder attrBuilder(moduleOp.getContext());
    moduleOp.walk([&](func::CallOp callInstruction) {
      StringRef targetFunctionName = callInstruction.getCallee();
      int totalCalls = functionCallFrequencyMap[targetFunctionName];
      IntegerAttr callCountAttribute =
          attrBuilder.getI64IntegerAttr(totalCalls);
      callInstruction->setAttr("call_count", callCountAttribute);
    });
  }

  StringRef getArgument() const final {
    return "FunctionCallCounterPass_Suvorov_Dmitrii_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "Counts how many times each function is called and adds this info "
           "as an attribute to func.call operations.";
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(FunctionCallCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(FunctionCallCounterPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "FunctionCallCounterPass", "1.0",
          []() { PassRegistration<FunctionCallCounterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}