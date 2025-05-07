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
private: // declaration
  static constexpr StringRef traceBegin = "trace_loop_iter_begin";
  static constexpr StringRef traceEnd = "trace_loop_iter_end";

  void ensureTraceFunctionExists(ModuleOp module, StringRef name) {
    if (module.lookupSymbol<func::FuncOp>(name))
      return;

    OpBuilder builder(module.getContext());
    builder.setInsertionPointToStart(module.getBody());

    auto func = builder.create<func::FuncOp>(module.getLoc(), name,
                                             builder.getFunctionType({}, {}));
    func.setSymVisibility("private");
  }

  void declareTraceFunctions(ModuleOp module) {

    ensureTraceFunctionExists(module, traceBegin);
    ensureTraceFunctionExists(module, traceEnd);
  }

private: // insertion
  void insertLoopTraceCalls(Block &body, Location loc) {
    OpBuilder b(body.getParent()->getContext());

    b.setInsertionPointToStart(&body);
    b.create<func::CallOp>(loc, traceBegin, TypeRange{}, ValueRange{});

    Operation *term = body.getTerminator();
    b.setInsertionPoint(term);
    b.create<func::CallOp>(loc, traceEnd, TypeRange{}, ValueRange{});
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
    ModuleOp module = getOperation();
    declareTraceFunctions(module);

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
