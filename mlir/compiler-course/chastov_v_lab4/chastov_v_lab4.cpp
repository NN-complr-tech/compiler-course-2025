#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
struct CeilTransformPattern : public OpRewritePattern<math::CeilOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(math::CeilOp op,
                                PatternRewriter &rewriter) const override {
    Location loc = op.getLoc();
    Value operand = op.getOperand();

    Value negated = rewriter.create<arith::NegFOp>(loc, operand);
    Value floored = rewriter.create<math::FloorOp>(loc, negated);
    Value result = rewriter.create<arith::NegFOp>(loc, floored);

    rewriter.replaceOp(op, result);
    return success();
  }
};

class CeilOptimizationPass
    : public PassWrapper<CeilOptimizationPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "CeilPlugin_Chastov_Vyacheslav_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Optimizes ceil operations via floor transformation";
  }

  void runOnOperation() override {
    Operation *module = getOperation();
    MLIRContext *context = &getContext();

    RewritePatternSet patterns(context);
    patterns.add<CeilTransformPattern>(context);

    if (failed(applyPatternsAndFoldGreedily(module, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CeilOptimizationPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CeilOptimizationPass)

PassPluginLibraryInfo getCeilOptPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CeilOptimization", "1.0",
          []() { mlir::PassRegistration<CeilOptimizationPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getCeilOptPassPluginInfo();
}
