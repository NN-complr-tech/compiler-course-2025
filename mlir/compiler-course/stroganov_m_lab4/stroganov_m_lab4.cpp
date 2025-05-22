#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {

class InsertLoopTracePass
    : public PassWrapper<InsertLoopTracePass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "InsertLoopTracePass_Stroganov_Mikhail_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Insert @trace_loop_iter_begin and @trace_loop_iter_end in loops";
  }

  void runOnOperation() override {
    auto func = getOperation();
    auto *ctx = &getContext();
    OpBuilder builder(ctx);

    if (!func.lookupSymbol<func::FuncOp>("trace_loop_iter_begin")) {
      builder.setInsertionPointToStart(func.getBody());
      builder
          .create<func::FuncOp>(func.getLoc(), "trace_loop_iter_begin",
                                builder.getFunctionType({}, {}))
          .setPrivate();
    }

    if (!func.lookupSymbol<func::FuncOp>("trace_loop_iter_end")) {
      builder.setInsertionPointToStart(func.getBody());
      builder
          .create<func::FuncOp>(func.getLoc(), "trace_loop_iter_end",
                                builder.getFunctionType({}, {}))
          .setPrivate();
    }

    func.walk([&](Operation *op) {
      if (isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ParallelOp>(
              op)) {
        if (auto forOp = dyn_cast<affine::AffineForOp>(op)) {
          insertTrace(forOp.getBody(), ctx, builder);
        } else if (auto forOp = dyn_cast<scf::ForOp>(op)) {
          insertTrace(forOp.getBody(), ctx, builder);
        } else if (auto whileOp = dyn_cast<scf::WhileOp>(op)) {
          insertTrace(&whileOp.getBefore().front(), ctx, builder);
          insertTrace(&whileOp.getAfter().front(), ctx, builder);
        } else if (auto parOp = dyn_cast<scf::ParallelOp>(op)) {
          insertTrace(parOp.getBody(), ctx, builder);
        }
      }
    });
  }

private:
  void insertTrace(Block *block, MLIRContext *ctx, OpBuilder &builder) {
    builder.setInsertionPointToStart(block);
    builder.create<func::CallOp>(block->getParentOp()->getLoc(),
                                 "trace_loop_iter_begin", TypeRange(),
                                 ValueRange{});

    Operation *terminator = block->getTerminator();
    builder.setInsertionPoint(terminator);
    builder.create<func::CallOp>(terminator->getLoc(), "trace_loop_iter_end",
                                 TypeRange(), ValueRange{});
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(InsertLoopTracePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(InsertLoopTracePass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "InsertLoopTracePass", "1.0",
          []() { mlir::PassRegistration<InsertLoopTracePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
