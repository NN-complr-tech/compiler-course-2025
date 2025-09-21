#include "mlir/IR/PatternMatch.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Pass/Pass.h"

using namespace mlir;

namespace {
struct FoldAddZeroPattern : OpRewritePattern<arith::AddIOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::AddIOp op, PatternRewriter &rewriter) const override {
    auto lhs = op.getLhs();
    auto rhs = op.getRhs();
    if (auto c = rhs.getDefiningOp<arith::ConstantOp>()) {
      if (auto intAttr = c.getValue().dyn_cast<IntegerAttr>()) {
        if (intAttr.getValue().isZero()) {
          rewriter.replaceOp(op, lhs);
          return success();
        }
      }
    }
    if (auto c = lhs.getDefiningOp<arith::ConstantOp>()) {
      if (auto intAttr = c.getValue().dyn_cast<IntegerAttr>()) {
        if (intAttr.getValue().isZero()) {
          rewriter.replaceOp(op, rhs);
          return success();
        }
      }
    }
    return failure();
  }
};

struct FoldAddZeroPass : PassWrapper<FoldAddZeroPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FoldAddZeroPass)
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<FoldAddZeroPattern>(&getContext());
    (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
  }
};
} // namespace

std::unique_ptr<Pass> createFoldAddZeroPass() {
  return std::make_unique<FoldAddZeroPass>();
}

// Registration for mlir-opt: -cc-fold-add-zero
namespace {
#define GEN_PASS_REGISTRATION
} // namespace

static PassRegistration<FoldAddZeroPass> pass(
    "cc-fold-add-zero",
    "Fold arith.addi x, 0 to x"
);
