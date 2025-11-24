#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
struct CeilToNegFloorPattern : public OpRewritePattern<math::CeilOp> {
  using OpRewritePattern<math::CeilOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(math::CeilOp op,
                                PatternRewriter &rewriter) const override {
    Location loc = op.getLoc();
    Value input = op.getOperand();

    Value neg = rewriter.create<arith::NegFOp>(loc, input);
    Value floor = rewriter.create<math::FloorOp>(loc, neg);
    Value result = rewriter.create<arith::NegFOp>(loc, floor);

    rewriter.replaceOp(op, result);
    return success();
  }
};

class ReplaceMathCeil
    : public PassWrapper<ReplaceMathCeil, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "ReplaceMathCeil_Polyakov_Alexey_FIIT2_MLIR";
  }

  StringRef getDescription() const final { return "Replace math.ceil"; }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *ctx = module.getContext();

    RewritePatternSet patterns(ctx);
    patterns.add<CeilToNegFloorPattern>(ctx);
    if (failed(applyPatternsAndFoldGreedily(module, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ReplaceMathCeil)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ReplaceMathCeil)

mlir::PassPluginLibraryInfo getReplaceMathCeilPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ReplaceMathCeil", "1.0",
          []() { PassRegistration<ReplaceMathCeil>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getReplaceMathCeilPluginInfo();
}