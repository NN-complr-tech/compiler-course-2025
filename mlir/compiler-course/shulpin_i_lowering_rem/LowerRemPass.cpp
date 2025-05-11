#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {
class LowerRemPass
    : public mlir::PassWrapper<LowerRemPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const final {
    return "LowerRemPass_ShulpinIlya_FIIT1_MLIR";
  }

  mlir::StringRef getDescription() const final {
    return "Lower arith.remsi and arith.remui into a - (a // b) * b";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();

    module.walk([&](mlir::arith::RemSIOp remOp) {
      RemLowering<mlir::arith::RemSIOp, mlir::arith::DivSIOp>().lower(remOp);
    });

    module.walk([&](mlir::arith::RemUIOp remOp) {
      RemLowering<mlir::arith::RemUIOp, mlir::arith::DivUIOp>().lower(remOp);
    });
  }

private:
  template <typename RemOp, typename DivOp>
  struct RemLowering {
    void lower(RemOp remOp) {
      mlir::OpBuilder builder(remOp);
      mlir::Value lhs = remOp.getLhs();
      mlir::Value rhs = remOp.getRhs();

      mlir::Value div = builder.create<DivOp>(remOp.getLoc(), lhs, rhs);
      mlir::Value mul =
          builder.create<mlir::arith::MulIOp>(remOp.getLoc(), div, rhs);
      mlir::Value result =
          builder.create<mlir::arith::SubIOp>(remOp.getLoc(), lhs, mul);

      remOp.replaceAllUsesWith(result);
      remOp.erase();
    }
  };
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LowerRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LowerRemPass)

mlir::PassPluginLibraryInfo getLowerRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LowerRemPass", "1.0",
          []() { mlir::PassRegistration<LowerRemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLowerRemPassPluginInfo();
}
