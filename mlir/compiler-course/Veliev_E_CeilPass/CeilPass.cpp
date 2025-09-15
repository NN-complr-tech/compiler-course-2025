#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace {
class ReplaceCeilPass
    : public mlir::PassWrapper<ReplaceCeilPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  llvm::StringRef getArgument() const final {
    return "ReplaceCeilPass_VelievElvin_FIIT1_MLIR";
  }
  llvm::StringRef getDescription() const final {
    return "Replace math.ceil with -math.floor(-x)";
  }

  void runOnOperation() override {
    mlir::MLIRContext *ctx = &getContext();
    mlir::RewritePatternSet patterns(ctx);
    patterns.add<ReplaceCeilPattern>(ctx);
    if (mlir::failed(mlir::applyPatternsAndFoldGreedily(getOperation(),
                                                        std::move(patterns)))) {
      signalPassFailure();
    }
  }

private:
  struct ReplaceCeilPattern
      : public mlir::OpRewritePattern<mlir::math::CeilOp> {
    using mlir::OpRewritePattern<mlir::math::CeilOp>::OpRewritePattern;

    template <typename NegOp, typename FloorOp>
    static mlir::Value createNegFloorNeg(mlir::PatternRewriter &rewriter,
                                         mlir::Location loc, mlir::Type type,
                                         mlir::Value input) {
      mlir::Value n1 = rewriter.create<NegOp>(loc, type, input).getResult();
      mlir::Value f = rewriter.create<FloorOp>(loc, type, n1).getResult();
      return rewriter.create<NegOp>(loc, type, f).getResult();
    }

    mlir::LogicalResult
    matchAndRewrite(mlir::math::CeilOp op,
                    mlir::PatternRewriter &rewriter) const override {
      mlir::Location loc = op.getLoc();
      mlir::Value input = op.getOperand();
      mlir::Type inType = input.getType();

      if (mlir::isa<mlir::IntegerType>(inType)) {
        rewriter.replaceOp(op, input);
        return mlir::success();
      }

      mlir::Value result =
          createNegFloorNeg<mlir::arith::NegFOp, mlir::math::FloorOp>(
              rewriter, loc, inType, input);

      rewriter.replaceOp(op, result);
      return mlir::success();
    }
  };
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ReplaceCeilPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ReplaceCeilPass)

mlir::PassPluginLibraryInfo getReplaceCeilPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ReplaceCeilPass", "1.0",
          []() { mlir::PassRegistration<ReplaceCeilPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getReplaceCeilPluginInfo();
}