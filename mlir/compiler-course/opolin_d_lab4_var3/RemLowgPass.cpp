#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class RemLowPass : public PassWrapper<RemLowPass, OperationPass<func::FuncOp>> {
public:
  StringRef getArgument() const final {
    return "RemLowPass_Opolin_Dmitry_FIIT2_MLIR";
  }
  StringRef getDescription() const final {
    return "Expands arith.remsi and arith.remui using direct IR (a - "
           "(a//b)*b).";
  }

  void runOnOperation() override {
    func::FuncOp func = getOperation();
    func.walk([&](arith::RemSIOp remOp) {
      OpBuilder builder(remOp);
      transformRemainderOperation<arith::DivSIOp>(remOp, builder);
    });
    func.walk([&](arith::RemUIOp remOp) {
      OpBuilder builder(remOp);
      transformRemainderOperation<arith::DivUIOp>(remOp, builder);
    });
  }

private:
  template <typename CorrespondingDivOpType, typename RemOpType>
  void transformRemainderOperation(RemOpType remOp, OpBuilder &builder) {
    Location loc = remOp.getLoc();
    Value dividend = remOp.getLhs();
    Value divisor = remOp.getRhs();

    Value divResult =
        builder.create<CorrespondingDivOpType>(loc, dividend, divisor);
    Value mulResult = builder.create<arith::MulIOp>(loc, divResult, divisor);
    Value subResult = builder.create<arith::SubIOp>(loc, dividend, mulResult);

    remOp.getResult().replaceAllUsesWith(subResult);
    remOp.erase();
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(RemLowPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(RemLowPass)

mlir::PassPluginLibraryInfo getRemLowPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "RemLowPass", "1.0",
          []() { mlir::PassRegistration<RemLowPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getRemLowPassPluginInfo();
}
