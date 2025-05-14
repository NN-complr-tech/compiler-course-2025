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
  template <typename RemOpType>
  struct RemXIOpRewrite : public OpRewritePattern<RemOpType> {

    template <typename RemXIOp>
    struct DivOpType {
      using Type = DivSIOp;
    };

    template <>
    struct DivOpType<RemUIOp> {
      using Type = DivUIOp;
    };

    using OpRewritePattern<RemOpType>::OpRewritePattern;
    LogicalResult matchAndRewrite(RemOpType operation,
                                  PatternRewriter &rw) const override {
      Location location = operation.getLoc();
      Value val1 = operation.getLhs();
      Value val2 = operation.getRhs();

      // Use the appropriate division operation based on the type
      Value division =
          rw.create<typename DivOpType<RemOpType>::Type>(location, val1, val2);
      Value multiplication = rw.create<MulIOp>(location, division, val2);
      Value substract = rw.create<SubIOp>(location, val1, multiplication);

      rw.replaceOp(operation, substract);
      return success();
    }
  };

public:
  StringRef getArgument() const final {
    return "rem_pass_Mironov_Arseniy_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "This pass breaks operations arith.remsi and arith.remui into "
           "calculation following the rule:\n rem(a, b) = a - (a / b) * b";
  }

  void runOnOperation() override {
    using RemSIRewritePattern = RemXIOpRewrite<RemSIOp>;
    using RemUIRewritePattern = RemXIOpRewrite<RemUIOp>;
    RewritePatternSet rewrite_patterns(&getContext());
    rewrite_patterns.add<RemSIRewritePattern>(&getContext());
    rewrite_patterns.add<RemUIRewritePattern>(&getContext());
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

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "rem_pass", "1.0",
          []() { mlir::PassRegistration<RemPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
