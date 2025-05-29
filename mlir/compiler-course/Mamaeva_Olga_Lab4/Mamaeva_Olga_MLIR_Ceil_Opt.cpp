#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
struct CeilToFloorPattern : public OpRewritePattern<math::CeilOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(math::CeilOp op,
                                PatternRewriter &rewriter) const override {
    Value neg = rewriter.create<arith::NegFOp>(op.getLoc(), op.getOperand());
    Value floor = rewriter.create<math::FloorOp>(op.getLoc(), neg);
    rewriter.replaceOp(op, rewriter.create<arith::NegFOp>(op.getLoc(), floor));
    return success();
  }
};

class CeilOptPass : public PassWrapper<CeilOptPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "CeilOpt_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Ceil to floor transformation";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<CeilToFloorPattern>(&getContext());
    (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(CeilOptPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(CeilOptPass)

PassPluginLibraryInfo getCeilOptPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "CeilOpt", "1.0", // Короткое имя плагина
          []() { PassRegistration<CeilOptPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getCeilOptPluginInfo();
}
