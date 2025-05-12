#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace arith;

namespace {

class RemPass : public PassWrapper<RemPass, OperationPass<ModuleOp>> {
private:
  struct RemSIOpRewrite : public OpRewritePattern<RemSIOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(RemSIOp operation,
                                  PatternRewriter &rewriter) const override {
      Location location = operation.getLoc();
      Value lhs = operation.getLhs();
      Value rhs = operation.getRhs();

      Value div = rewriter.create<DivSIOp>(location, lhs, rhs);
      Value mul = rewriter.create<MulIOp>(location, div, rhs);
      Value sub = rewriter.create<SubIOp>(location, lhs, mul);

      rewriter.replaceOp(operation, sub);
      return success();
    }
  };

  struct RemUIOpRewrite : public OpRewritePattern<RemUIOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(RemUIOp operation,
                                  PatternRewriter &rewriter) const override {
      Location location = operation.getLoc();
      Value lhs = operation.getLhs();
      Value rhs = operation.getRhs();

      Value div = rewriter.create<DivUIOp>(location, lhs, rhs);
      Value mul = rewriter.create<MulIOp>(location, div, rhs);
      Value sub = rewriter.create<SubIOp>(location, lhs, mul);

      rewriter.replaceOp(operation, sub);
      return success();
    }
  };

public:
  StringRef getArgument() const final { return "rem-pass-mamaeva-olga-fiit3"; }

  StringRef getDescription() const final {
    return "Replace remui/remsi with equivalent operations using div+mul+sub";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<RemSIOpRewrite, RemUIOpRewrite>(&getContext());

    if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                            std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass)

mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemPassMamaevaOlgaFIIT3", "1.0",
          []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
