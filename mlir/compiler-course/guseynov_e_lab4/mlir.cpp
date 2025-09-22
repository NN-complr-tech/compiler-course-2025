#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace {

template <typename RemainderOp, typename DivisionOp>
class RemainderExpansionPattern : public mlir::OpRewritePattern<RemainderOp> {
public:
  using mlir::OpRewritePattern<RemainderOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(RemainderOp remainderOperation, 
                  mlir::PatternRewriter &patternRewriter) const override {
    auto operationLocation = remainderOperation.getLoc();
    auto leftOperand = remainderOperation.getLhs();
    auto rightOperand = remainderOperation.getRhs();

    // Check for division by zero constant
    if (auto constantDivisor = llvm::dyn_cast_or_null<mlir::arith::ConstantIntOp>(
            rightOperand.getDefiningOp())) {
      if (constantDivisor.value() == 0) {
        return patternRewriter.notifyMatchFailure(remainderOperation,
                                           "cannot divide by zero (constant)");
      }
    }

    auto divisionResult = patternRewriter.create<DivisionOp>(
        operationLocation, leftOperand, rightOperand);
    
    auto multiplicationResult = patternRewriter.create<mlir::arith::MulIOp>(
        operationLocation, divisionResult, rightOperand);
    
    auto subtractionResult = patternRewriter.create<mlir::arith::SubIOp>(
        operationLocation, leftOperand, multiplicationResult);

    patternRewriter.replaceOp(remainderOperation, subtractionResult);
    return mlir::success();
  }
};

class IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR
    : public mlir::PassWrapper<IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  llvm::StringRef getArgument() const final {
    return "IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR";
  }

  llvm::StringRef getDescription() const final {
    return "Expands remainder operations into division-multiplication-subtraction sequence";
  }

  void runOnOperation() override {
    mlir::RewritePatternSet patternCollection(&getContext());
    
    patternCollection.add<
        RemainderExpansionPattern<mlir::arith::RemSIOp, mlir::arith::DivSIOp>,
        RemainderExpansionPattern<mlir::arith::RemUIOp, mlir::arith::DivUIOp>
    >(&getContext());

    if (mlir::failed(mlir::applyPatternsAndFoldGreedily(
          getOperation(), std::move(patternCollection)))) {
      signalPassFailure();
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR)
MLIR_DEFINE_EXPLICIT_TYPE_ID(IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR)

mlir::PassPluginLibraryInfo getRemDecompositionPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "IntegerRemainderExpansion", "1.0",
          []() { mlir::PassRegistration<IntegerRemainderExpansion_Guseynov_Emil_FIIT2_MLIR>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemDecompositionPassPluginInfo();
}