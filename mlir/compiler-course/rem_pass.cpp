#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }
  StringRef getDescription() const final { return "Replace remainder ops"; }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    module.walk([&](arith::RemSIOp op) {
      builder.setInsertionPoint(op);
      Value lhs = op.getLhs();
      Value rhs = op.getRhs();
      Location loc = op.getLoc();

      Value div = builder.create<arith::DivSIOp>(loc, lhs, rhs);
      Value mul = builder.create<arith::MulIOp>(loc, div, rhs);
      Value result = builder.create<arith::SubIOp>(loc, lhs, mul);

      op.getResult().replaceAllUsesWith(result);
      op.erase();
    });

    module.walk([&](arith::RemUIOp op) {
      builder.setInsertionPoint(op);
      Value lhs = op.getLhs();
      Value rhs = op.getRhs();
      Location loc = op.getLoc();

      Value div = builder.create<arith::DivUIOp>(loc, lhs, rhs);
      Value mul = builder.create<arith::MulIOp>(loc, div, rhs);
      Value result = builder.create<arith::SubIOp>(loc, lhs, mul);

      op.getResult().replaceAllUsesWith(result);
      op.erase();
    });
  }
};
} // namespace

void registerMamaevaRemPass() { PassRegistration<MamaevaRemPass>(); }

extern "C" LLVM_ATTRIBUTE_WEAK ::mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3_MLIR", "1.0",
          &registerMamaevaRemPass};
}
