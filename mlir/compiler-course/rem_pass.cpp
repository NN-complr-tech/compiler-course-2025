#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

template <typename RemOp, typename DivOp>
class RemReplacementPattern : public OpRewritePattern<RemOp> {
public:
  using OpRewritePattern<RemOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(RemOp remOp,
                                PatternRewriter &rewriter) const override {
    Location loc = remOp.getLoc();
    Value lhs = remOp.getLhs();
    Value rhs = remOp.getRhs();

    if (auto rhsConst =
            dyn_cast_or_null<arith::ConstantIntOp>(rhs.getDefiningOp())) {
      if (rhsConst.value() == 0) {
        return rewriter.notifyMatchFailure(remOp,
                                           "division by zero (constant)");
      }
    }

    Value div = rewriter.create<DivOp>(loc, lhs, rhs);
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs);
    Value sub = rewriter.create<arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(remOp, sub);
    return success();
  }
};

class RemPass : public PassWrapper<RemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "rem-pass"; }

  StringRef getDescription() const final {
    return "Decomposes remainder operations into div+mul+sub sequences";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<RemReplacementPattern<arith::RemSIOp, arith::DivSIOp>,
                 RemReplacementPattern<arith::RemUIOp, arith::DivUIOp>>(
        &getContext());

    if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                            std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass)

mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemPass", "1.0",
          []() { PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
