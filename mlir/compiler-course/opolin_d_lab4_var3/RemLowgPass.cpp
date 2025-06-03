#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;
using namespace mlir::arith;

namespace {
class RemLowPass : public PassWrapper<RemLowPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "RemLowPass_Opolin_Dmitry_FIIT2_MLIR";
  }
  StringRef getDescription() const final {
    return "Expands arith.remsi and arith.remui using direct IR (a - "
           "(a//b)*b).";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    module.walk([&](RemSIOp remOp) {
      OpBuilder builder(remOp);
      transformRemainderOperation<DivSIOp>(remOp, builder);
    });
    module.walk([&](RemUIOp remOp) {
      OpBuilder builder(remOp);
      transformRemainderOperation<DivUIOp>(remOp, builder);
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
    Value mulResult = builder.create<MulIOp>(loc, divResult, divisor);
    Value subResult = builder.create<SubIOp>(loc, dividend, mulResult);

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
