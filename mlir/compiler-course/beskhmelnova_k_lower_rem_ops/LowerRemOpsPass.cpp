#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
template <typename RemOp, typename DivOp>
struct RemRewritePattern : public OpRewritePattern<RemOp> {
  using OpRewritePattern<RemOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(RemOp op,
                                PatternRewriter &rewriter) const override {
    Location loc = op.getLoc();
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();

    Value div = rewriter.create<DivOp>(loc, lhs, rhs);
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs);
    Value res = rewriter.create<arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(op, res);
    return success();
  }
};

using RemSIRewritePattern = RemRewritePattern<arith::RemSIOp, arith::DivSIOp>;
using RemUIRewritePattern = RemRewritePattern<arith::RemUIOp, arith::DivUIOp>;

class LowerRemOpsPass
    : public PassWrapper<LowerRemOpsPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "LowerRemOpsPass_Beskhmelnova_Kseniya_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "Lower arith.rem{si,ui} to arithmetic expressions";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    MLIRContext *ctx = module.getContext();
    RewritePatternSet patterns(ctx);

    patterns.add<RemSIRewritePattern, RemUIRewritePattern>(ctx);
    (void)applyPatternsAndFoldGreedily(module, std::move(patterns));
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LowerRemOpsPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LowerRemOpsPass)

mlir::PassPluginLibraryInfo getLowerRemOpsPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LowerRemOpsPass", "1.0",
          []() { PassRegistration<LowerRemOpsPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLowerRemOpsPassPluginInfo();
}
