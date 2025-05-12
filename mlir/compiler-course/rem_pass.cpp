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
                                  PatternRewriter &rw) const override {
      Location location = operation.getLoc();

      Value val1 = operation.getLhs();
      Value val2 = operation.getRhs();
      Value division = rw.create<DivSIOp>(location, val1, val2);
      Value multiplication = rw.create<MulIOp>(location, division, val2);
      Value substract = rw.create<SubIOp>(location, val1, multiplication);

      rw.replaceOp(operation, substract);
      return success();
    }
  };

  struct RemUIOpRewrite : public OpRewritePattern<RemUIOp> {
    using OpRewritePattern::OpRewritePattern;

    LogicalResult matchAndRewrite(RemUIOp operation,
                                  PatternRewriter &rw) const override {
      Location location = operation.getLoc();

      Value val1 = operation.getLhs();
      Value val2 = operation.getRhs();
      Value division = rw.create<DivUIOp>(location, val1, val2);
      Value multiplication = rw.create<MulIOp>(location, division, val2);
      Value substract = rw.create<SubIOp>(location, val1, multiplication);

      rw.replaceOp(operation, substract);
      return success();
    }
  };

public:
  StringRef getArgument() const final {
    return "rem_pass_Mamaeva_Olga_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "This pass breaks operations arith.remsi and arith.remui into "
           "calculation following the rule:\n rem(a, b) = a - (a / b) * b";
  }

  void runOnOperation() override {
    RewritePatternSet rewrite_patterns(&getContext());
    rewrite_patterns.add<RemSIOpRewrite, RemUIOpRewrite>(&getContext());
    LogicalResult result = applyPatternsAndFoldGreedily(
        getOperation(), std::move(rewrite_patterns), GreedyRewriteConfig(),
        nullptr);

    if (failed(result)) {
      llvm::errs() << "Something went wrong.\n";
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemPass)

mlir::PassPluginLibraryInfo getRemPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass_Mamaeva_Olga_FIIT3", "1.0",
          []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemPassPluginInfo();
}
