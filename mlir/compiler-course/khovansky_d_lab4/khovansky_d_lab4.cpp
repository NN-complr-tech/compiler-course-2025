#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class RemsiuiPass : public PassWrapper<RemsiuiPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "RemsiuiPass_Khovansky_Dmitry_FIIT2_MLIR";
  }
  StringRef getDescription() const final {
    return "Decomposes arith.remsi and arith.remui operations into equivalent "
           "expressions using division and multiplication: a - (a // b) * b";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();

    SmallVector<Operation *, 8> remOps;

    moduleOp.walk([&](Operation *op) {
      if (isa<arith::RemSIOp>(op) || isa<arith::RemUIOp>(op)) {
        remOps.push_back(op);
      }
    });

    for (Operation *op : remOps) {
      OpBuilder builder(op);
      Location loc = op->getLoc();

      // rem(a, b) = a - (a // b) * b
      Value a, b, div;

      if (auto remsi = dyn_cast<arith::RemSIOp>(op)) {
        a = remsi.getLhs();
        b = remsi.getRhs();
        
        div = builder.create<arith::DivSIOp>(loc, a, b);
      } else if (auto remui = dyn_cast<arith::RemUIOp>(op)) {
        a = remui.getLhs();
        b = remui.getRhs();

        div = builder.create<arith::DivUIOp>(loc, a, b);
      } else {
        continue;
      }

      Value mul = builder.create<arith::MulIOp>(loc, div, b);
      Value result = builder.create<arith::SubIOp>(loc, a, mul);

      op->getResult(0).replaceAllUsesWith(result);
      op->erase();
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemsiuiPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemsiuiPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemsiuiPass", "1.0",
          []() { mlir::PassRegistration<RemsiuiPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
