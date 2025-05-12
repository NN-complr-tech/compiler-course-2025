#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<func::FuncOp>> {
private:
  void expandRemainder(Operation *op, PatternRewriter &rewriter,
                       bool isSigned) {
    Value lhs = op->getOperand(0);
    Value rhs = op->getOperand(1);
    Location loc = op->getLoc();

    Value div =
        isSigned ? rewriter.create<arith::DivSIOp>(loc, lhs, rhs).getResult()
                 : rewriter.create<arith::DivUIOp>(loc, lhs, rhs).getResult();
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs).getResult();
    Value result = rewriter.create<arith::SubIOp>(loc, lhs, mul).getResult();

    rewriter.replaceOp(op, result);
  }

public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Replace remainder ops with div+mul+sub sequence";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add([&](Operation *op) {
      if (auto remsi = dyn_cast<arith::RemSIOp>(op)) {
        expandRemainder(remsi, PatternRewriter(op->getContext()), true);
        return success();
      }
      if (auto remui = dyn_cast<arith::RemUIOp>(op)) {
        expandRemainder(remui, PatternRewriter(op->getContext()), false);
        return success();
      }
      return failure();
    });

    if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                            std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3", "1.0",
          [](PassRegistry &registry) {
            registry.addPass(std::make_unique<MamaevaRemPass>());
          }};
}
