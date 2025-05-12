#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

#define PLUGIN_NAME "rem_pass_Mamaeva_Olga_FIIT3_MLIR"

using namespace mlir;
using namespace arith;

namespace {

class MamaevaRemPass
    : public PassWrapper<MamaevaRemPass, OperationPass<ModuleOp>> {
private:
  struct SignedRemRewriter : public OpRewritePattern<RemSIOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(RemSIOp op,
                                  PatternRewriter &rewriter) const override {
      Value lhs = op.getLhs();
      Value rhs = op.getRhs();
      Location loc = op.getLoc();

      Value div = rewriter.create<DivSIOp>(loc, lhs, rhs);
      Value mul = rewriter.create<MulIOp>(loc, div, rhs);
      Value result = rewriter.create<SubIOp>(loc, lhs, mul);

      rewriter.replaceOp(op, result);
      return success();
    }
  };

  struct UnsignedRemRewriter : public OpRewritePattern<RemUIOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(RemUIOp op,
                                  PatternRewriter &rewriter) const override {
      Value lhs = op.getLhs();
      Value rhs = op.getRhs();
      Location loc = op.getLoc();

      Value div = rewriter.create<DivUIOp>(loc, lhs, rhs);
      Value mul = rewriter.create<MulIOp>(loc, div, rhs);
      Value result = rewriter.create<SubIOp>(loc, lhs, mul);

      rewriter.replaceOp(op, result);
      return success();
    }
  };

public:
  StringRef getArgument() const final { return PLUGIN_NAME; }

  StringRef getDescription() const final {
    return "Replace remainder ops with equivalent arithmetic operations";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<SignedRemRewriter, UnsignedRemRewriter>(&getContext());

    if (failed(applyPatternsAndFoldGreedily(getOperation(), std::move(patterns))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

namespace {
mlir::PassPluginLibraryInfo getMamaevaRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, PLUGIN_NAME, "1.0",
          []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaRemPassPluginInfo();
}
