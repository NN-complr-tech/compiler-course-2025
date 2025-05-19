#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

class RemLoweringPattern : public OpRewritePattern<arith::RemSIOp> {
public:
  RemLoweringPattern(MLIRContext *context)
      : OpRewritePattern<arith::RemSIOp>(context) {}

  LogicalResult matchAndRewrite(arith::RemSIOp op,
                                PatternRewriter &rewriter) const override {
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();
    Location loc = op.getLoc();

    Value div = rewriter.create<arith::DivSIOp>(loc, lhs, rhs);
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs);
    Value sub = rewriter.create<arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(op, sub);
    return success();
  }
};

class URemLoweringPattern : public OpRewritePattern<arith::RemUIOp> {
public:
  URemLoweringPattern(MLIRContext *context)
      : OpRewritePattern<arith::RemUIOp>(context) {}

  LogicalResult matchAndRewrite(arith::RemUIOp op,
                                PatternRewriter &rewriter) const override {
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();
    Location loc = op.getLoc();

    Value div = rewriter.create<arith::DivUIOp>(loc, lhs, rhs);
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs);
    Value sub = rewriter.create<arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(op, sub);
    return success();
  }
};

class MamaevaRemPass : public PassWrapper<MamaevaRemPass, OperationPass<>> {
public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Replace remainder ops with div+mul+sub sequence";
  }

  void runOnOperation() override {
    Operation *op = getOperation();
    MLIRContext *context = op->getContext();

    RewritePatternSet patterns(context);
    patterns.add<RemLoweringPattern, URemLoweringPattern>(context);

    if (failed(applyPatternsAndFoldGreedily(op, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MamaevaRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MamaevaRemPass)

namespace {
mlir::PassPluginLibraryInfo getMamaevaRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3", "1.0",
          []() { mlir::PassRegistration<MamaevaRemPass>(); }};
}
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMamaevaRemPassPluginInfo();
}
