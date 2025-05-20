#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {
template <typename ModOp, typename DivOp>
struct ModRewritePattern : public OpRewritePattern<ModOp> {
  using OpRewritePattern<ModOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(ModOp operation,
                                PatternRewriter &builder) const override {
    Location location = operation.getLoc();
    Value leftOperand = operation.getLhs();
    Value rightOperand = operation.getRhs();

    Value division = builder.create<DivOp>(location, leftOperand, rightOperand);
    Value multiplication =
        builder.create<arith::MulIOp>(location, division, rightOperand);
    Value result =
        builder.create<arith::SubIOp>(location, leftOperand, multiplication);

    builder.replaceOp(operation, result);
    return success();
  }
};

using ModSIRewritePattern = ModRewritePattern<arith::RemSIOp, arith::DivSIOp>;
using ModUIRewritePattern = ModRewritePattern<arith::RemUIOp, arith::DivUIOp>;

class LowerModOpsPass
    : public PassWrapper<LowerModOpsPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "LowerModOpsPass_Yasakova_Tanya_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Lower arith.rem{si,ui} to arithmetic expressions";
  }

  void runOnOperation() override {
    ModuleOp mod = getOperation();
    MLIRContext *context = mod.getContext();
    RewritePatternSet patternSet(context);

    patternSet.add<ModSIRewritePattern, ModUIRewritePattern>(context);
    (void)applyPatternsAndFoldGreedily(mod, std::move(patternSet));
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LowerModOpsPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LowerModOpsPass)

mlir::PassPluginLibraryInfo getLowerModOpsPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LowerModOpsPass", "1.0",
          []() { PassRegistration<LowerModOpsPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLowerModOpsPassPluginInfo();
}
