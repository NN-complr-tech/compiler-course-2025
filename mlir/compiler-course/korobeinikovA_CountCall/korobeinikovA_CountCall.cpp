#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class ExamplePass : public PassWrapper<ExamplePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "ExamplePass_Ivanov_Ivan_FIIT0_MLIR";
  }
  StringRef getDescription() const final { return "Description pass"; }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    auto countOp = 0;
    moduleOp.walk([&](Operation *op) { ++countOp; });

    llvm::outs() << "Count operations: " << countOp << '\n';
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ExamplePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ExamplePass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ExamplePass", "1.0",
          []() { mlir::PassRegistration<ExamplePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
