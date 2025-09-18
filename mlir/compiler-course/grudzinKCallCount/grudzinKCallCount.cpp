#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
struct CallCountPass
    : public PassWrapper<CallCountPass, OperationPass<ModuleOp>> {
  StringRef getArgument() const final {
    return "CallCountPass_Grudzin_Konstantin_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "Count calls of each function and attach call_count attribute to "
           "func.call ops";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    llvm::StringMap<int64_t> counts;
    module.walk([&](func::CallOp callOp) {
      StringRef callee = callOp.getCallee();
      counts[callee] += 1;
    });

    OpBuilder builder(module.getContext());
    module.walk([&](func::CallOp callOp) {
      StringRef callee = callOp.getCallee();
      int64_t totalCount = counts.lookup(callee);
      IntegerAttr countAttr = builder.getI64IntegerAttr(totalCount);
      callOp->setAttr("call_count", countAttr);
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CallCountPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CallCountPass)

static PassPluginLibraryInfo getCallCountPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CallCountPass", "1.0",
          []() { mlir::PassRegistration<CallCountPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getCallCountPassPluginInfo();
}
