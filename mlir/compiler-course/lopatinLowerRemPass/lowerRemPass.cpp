#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
// rem(a,b) -> a - (a // b) * b
template <typename RemOp, typename DivOp>
struct GenericRemLowering : public OpRewritePattern<RemOp> {
  GenericRemLowering(MLIRContext *context) : OpRewritePattern<RemOp>(context) {}
  LogicalResult matchAndRewrite(RemOp op,
                                PatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    Value a = op.getOperand(0);
    Value b = op.getOperand(1);
    auto divOp = rewriter.create<DivOp>(loc, a, b);
    Value quo = divOp.getResult();
    auto mulOp = rewriter.create<arith::MulIOp>(loc, quo, b);
    auto subOp = rewriter.create<arith::SubIOp>(loc, a, mulOp.getResult());
    rewriter.replaceOp(op, subOp.getResult());
    return success();
  }
};

using RemSIOpLowering = GenericRemLowering<arith::RemSIOp, arith::DivSIOp>;
using RemUIOpLowering = GenericRemLowering<arith::RemUIOp, arith::DivUIOp>;

struct LowerRemPass
    : public PassWrapper<LowerRemPass, OperationPass<ModuleOp>> {
  StringRef getArgument() const final {
    return "LowerRemPass_IlyaLopatin_FIIT3_MLIR";
  }
  StringRef getDescription() const final {
    return "Lower arith.remsi/remui into a - (a // b) * b";
  }
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<arith::ArithDialect>();
  }
  void runOnOperation() override {
    MLIRContext &ctx = getContext();
    RewritePatternSet patterns(&ctx);
    patterns.add<RemSIOpLowering, RemUIOpLowering>(&ctx);
    (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LowerRemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LowerRemPass)

mlir::PassPluginLibraryInfo getLowerRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LowerRemPass", "1.0",
          []() { mlir::PassRegistration<LowerRemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLowerRemPassPluginInfo();
}
