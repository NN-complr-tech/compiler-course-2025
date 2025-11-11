#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/CallInterfaces.h"
#include "mlir/Pass/Pass.h"
#include "llvm/ADT/StringMap.h"

using namespace mlir;

namespace {
class GenericCallCounter
    : public PassWrapper<GenericCallCounter, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "generic-call-counter-IvashchukVA";
  }

  StringRef getDescription() const final { return "Count function calls"; }

  void runOnOperation() override {
    ModuleOp module = getOperation();

    llvm::StringMap<int64_t> callCounts;
    module.walk([&](CallOpInterface callOp) {
      if (auto symCall =
              callOp.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
        callCounts[symCall.getRootReference()]++;
      }
    });

    OpBuilder builder(module.getContext());
    module.walk([&](CallOpInterface callOp) {
      if (auto symCall =
              callOp.getCallableForCallee().dyn_cast<SymbolRefAttr>()) {
        int64_t count = callCounts.lookup(symCall.getRootReference());
        callOp->setAttr("call_count", builder.getIndexAttr(count));
      }
    });
  }
};
} // namespace

void registerGenericCallCounter() { PassRegistration<GenericCallCounter>(); }

extern "C" void mlirRegisterPasses() { registerGenericCallCounter(); }