#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

template <typename RemainderOp, typename DivisionOp>
struct RemainderExpansionPattern : public OpRewritePattern<RemainderOp> {
  explicit RemainderExpansionPattern(MLIRContext *context)
      : OpRewritePattern<RemainderOp>(context, /*benefit=*/1) {}

  LogicalResult matchAndRewrite(RemainderOp remOp,
                                PatternRewriter &rewriter) const override {
    Location loc = remOp.getLoc();

    Value dividend = remOp.getLhs();
    Value divisor = remOp.getRhs();

    Value quotient = rewriter.create<DivisionOp>(loc, dividend, divisor);

    Value product = rewriter.create<arith::MulIOp>(loc, quotient, divisor);

    Value finalResult = rewriter.create<arith::SubIOp>(loc, dividend, product);

    rewriter.replaceOp(remOp, finalResult);

    return success();
  }
};

using SignedRemainderExpansion =
    RemainderExpansionPattern<arith::RemSIOp, arith::DivSIOp>;
using UnsignedRemainderExpansion =
    RemainderExpansionPattern<arith::RemUIOp, arith::DivUIOp>;

struct DeconstructRemainderPass
    : public PassWrapper<DeconstructRemainderPass, OperationPass<ModuleOp>> {

  StringRef getArgument() const final {
    return "deconstruct-remainder-pass-by-ilya-lopatin";
  }

  StringRef getDescription() const final {
    return "Replaces integer remainder operations with their mathematical "
           "definition: a - (a / b) * b";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect>();
  }

  void runOnOperation() override {
    MLIRContext &context = getContext();
    RewritePatternSet patterns(&context);

    patterns.add<SignedRemainderExpansion, UnsignedRemainderExpansion>(
        &context);

    if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                            std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(DeconstructRemainderPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(DeconstructRemainderPass)

mlir::PassPluginLibraryInfo getDeconstructRemainderPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "DeconstructRemainder", "0.1.0",
          []() { mlir::PassRegistration<DeconstructRemainderPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getDeconstructRemainderPassPluginInfo();
}