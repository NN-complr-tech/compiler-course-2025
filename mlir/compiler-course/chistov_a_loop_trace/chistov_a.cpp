#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <functional>

using namespace mlir;
using namespace mlir::func;
using namespace mlir::scf;

namespace {
class LoopTracePass : public PassWrapper<LoopTracePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "LoopTracePass_Chistov_Alexey_FIIT1_MLIR";
  }
  StringRef getDescription() const final { 
    return "Instrument loops with iteration tracing calls."; 
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    declareTracingFunctions(moduleOp);

    moduleOp.walk([&](Operation *op) {
      if (auto forOp = llvm::dyn_cast<AffineForOp>(op)) {
        instrumentLoop(forOp, forOp.getBody(), builder);
      } else if (auto forOp = dyn_cast<ForOp>(op)) {
        instrumentLoop(forOp, forOp.getBody(), builder);
      } else if (auto whileOp = dyn_cast<WhileOp>(op)) {
        instrumentWhileLoop(whileOp, builder);
      }
    });
  }

private:
  void declareTracingFunctions(ModuleOp moduleOp) {
    OpBuilder builder(moduleOp);
    builder.setInsertionPointToStart(moduleOp.getBody());

    std::function<void(StringRef)> declareFunc = [&](StringRef name) {
      if (!moduleOp.lookupSymbol<FuncOp>(name)) {
        auto funcType = builder.getFunctionType({}, builder.getVoidType());
        builder.create<FuncOp>(moduleOp.getLoc(), name, funcType);
      }
    };

    declareFunc("trace_loop_iter_begin");
    declareFunc("trace_loop_iter_end");
  }

  template <typename LoopOp>
  void instrumentLoop(LoopOp loopOp, Block *body, OpBuilder &builder) {
    builder.setInsertionPointToStart(body);
    builder.create<CallOp>(loopOp.getLoc(), "trace_loop_iter_begin", TypeRange{});

    builder.setInsertionPoint(body->getTerminator());
    builder.create<CallOp>(loopOp.getLoc(), "trace_loop_iter_end", TypeRange{});
  }

  void instrumentWhileLoop(WhileOp whileOp, OpBuilder &builder) {
    builder.setInsertionPointToStart(&whileOp.getBefore().front());
    builder.create<CallOp>(whileOp.getLoc(), "trace_loop_iter_begin", TypeRange{});

    builder.setInsertionPoint(whileOp.getAfter().front().getTerminator());
    builder.create<CallOp>(whileOp.getLoc(), "trace_loop_iter_end", TypeRange{});
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopTracePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopTracePass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopTracePass", "1.0",
          []() { mlir::PassRegistration<LoopTracePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}