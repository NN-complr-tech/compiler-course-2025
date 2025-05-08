#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class LoopTracePass
    : public PassWrapper<LoopTracePass, OperationPass<ModuleOp>> {
  ModuleOp module;
  OpBuilder builder;

private: // declaration
  void ensureTraceFunctionExists(StringRef name) {
    if (SymbolTable::lookupSymbolIn(module, name))
      return;

    builder.setInsertionPointToStart(module.getBody());
    auto func = builder.create<func::FuncOp>(module.getLoc(), name,
                                             builder.getFunctionType({}, {}));
    func.setSymVisibility("private");
  }

  void declareTraceFunctions() {
    ensureTraceFunctionExists("trace_loop_iter_begin");
    ensureTraceFunctionExists("trace_loop_iter_end");
  }

private: // insertion
  void insertLoopTraceCalls(Block &body, Location loc) {
    builder.setInsertionPointToStart(&body);
    builder.create<func::CallOp>(loc, "trace_loop_iter_end", TypeRange{},
                                 ValueRange{});

    Operation *term = body.getTerminator();
    builder.setInsertionPoint(term);
    builder.create<func::CallOp>(loc, "trace_loop_iter_end", TypeRange{},
                                 ValueRange{});
  }

  void insertLoopTraceCallsForOp(Operation *op, Location loc) {
    if (auto affineFor = dyn_cast<affine::AffineForOp>(op)) {
      insertLoopTraceCalls(*affineFor.getBody(), loc);
      return;
    }

    if (auto scfFor = dyn_cast<scf::ForOp>(op)) {
      insertLoopTraceCalls(*scfFor.getBody(), loc);
      return;
    }

    if (auto scfWhile = dyn_cast<scf::WhileOp>(op)) {
      insertLoopTraceCalls(scfWhile.getAfter().front(), loc);
      return;
    }

    if (auto scfParallel = dyn_cast<scf::ParallelOp>(op)) {
      for (Region &region : scfParallel->getRegions()) {
        for (Block &block : region) {
          insertLoopTraceCalls(block, loc);
        }
      }
      return;
    }
  }

public:
  StringRef getArgument() const final { return "LoopTracePass"; }

  StringRef getDescription() const final {
    return "Insert @trace_loop_iter_begin at loop starts and "
           "@trace_loop_iter_end before exits";
  }

  void runOnOperation() override {
    module = getOperation();
    OpBuilder builder(module.getContext());

    declareTraceFunctions();
    module.walk([&](Operation *op) {
      Location loc = op->getLoc();
      insertLoopTraceCallsForOp(op, loc);
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopTracePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopTracePass)

mlir::PassPluginLibraryInfo getTraceLoopPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopTracePass", "1.0",
          []() { mlir::PassRegistration<LoopTracePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTraceLoopPassPluginInfo();
}
