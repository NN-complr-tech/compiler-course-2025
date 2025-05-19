#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace {

template <typename RemOp, typename DivOp>
class DecomposeRemOpPattern : public mlir::OpRewritePattern<RemOp> {
public:
  using mlir::OpRewritePattern<RemOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(RemOp remOp, mlir::PatternRewriter &rewriter) const override {
    auto loc = remOp.getLoc();
    auto lhs = remOp.getLhs();
    auto rhs = remOp.getRhs();

    if (auto rhsConst = llvm::dyn_cast_or_null<mlir::arith::ConstantIntOp>(
            rhs.getDefiningOp())) {
      if (rhsConst.value() == 0) {
        return rewriter.notifyMatchFailure(remOp,
                                           "division by zero (constant)");
      }
    }

    auto div = rewriter.create<DivOp>(loc, lhs, rhs);
    auto mul = rewriter.create<mlir::arith::MulIOp>(loc, div, rhs);
    auto sub = rewriter.create<mlir::arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(remOp, sub);
    return mlir::success();
  }
};

class RemDecompositionPass
    : public mlir::PassWrapper<RemDecompositionPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  llvm::StringRef getArgument() const final {
    return "RemDecomposePass_Frolova_Elizaveta_FIIT3_MLIR";
  }

  llvm::StringRef getDescription() const final {
    return "Decomposes arith operations into div and mul/sub.";
  }

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    patterns
        .add<DecomposeRemOpPattern<mlir::arith::RemSIOp, mlir::arith::DivSIOp>,
             DecomposeRemOpPattern<mlir::arith::RemUIOp, mlir::arith::DivUIOp>>(
            &getContext());

    if (mlir::failed(mlir::applyPatternsAndFoldGreedily(getOperation(),
                                                        std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemDecompositionPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemDecompositionPass)

mlir::PassPluginLibraryInfo getRemDecompositionPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemDecomposePass", "1.0",
          []() { mlir::PassRegistration<RemDecompositionPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemDecompositionPassPluginInfo();
}