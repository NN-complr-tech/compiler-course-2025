#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

template <typename RemOp, typename DivOp>
class RemReplacementPattern : public OpRewritePattern<RemOp> {
public:
  using OpRewritePattern<RemOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(RemOp remOp,
                                PatternRewriter &rewriter) const override {
    Location loc = remOp.getLoc();
    Value lhs = remOp.getLhs();
    Value rhs = remOp.getRhs();

    // Проверка деления на ноль для констант
    if (auto rhsConst =
            dyn_cast_or_null<arith::ConstantIntOp>(rhs.getDefiningOp())) {
      if (rhsConst.value() == 0) {
        return rewriter.notifyMatchFailure(remOp,
                                           "division by zero (constant)");
      }
    }

    Value div = rewriter.create<DivOp>(loc, lhs, rhs);
    Value mul = rewriter.create<arith::MulIOp>(loc, div, rhs);
    Value sub = rewriter.create<arith::SubIOp>(loc, lhs, mul);

    rewriter.replaceOp(remOp, sub);
    return success();
  }
};

class RemPass : public PassWrapper<RemPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final { return "rem-pass"; }
  StringRef getDescription() const final {
    return "Replace remainder operations with div+mul+sub";
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<RemReplacementPattern<arith::RemSIOp, arith::DivSIOp>,
                 RemReplacementPattern<arith::RemUIOp, arith::DivUIOp>>(
        &getContext());

    if (failed(applyPatternsAndFoldGreedily(getOperation(),
                                            std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

// Регистрация плагина
extern "C" LLVM_ATTRIBUTE_WEAK ::mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "RemPass", // Имя плагина
          "1.0", [](PassRegistry &registry) {
            registry.addPass(std::make_unique<RemPass>());
          }};
}
