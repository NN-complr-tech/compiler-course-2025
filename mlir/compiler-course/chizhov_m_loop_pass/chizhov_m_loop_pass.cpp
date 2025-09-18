#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
class TraceLoopIterPass
    : public PassWrapper<TraceLoopIterPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Inserts `@trace_loop_iter_begin` and `@trace_loop_iter_end` calls "
           "into each loop iteration";
  }

  void insertTraceCalls(Block &body, OpBuilder &builder, Location loc) {
    builder.setInsertionPointToStart(&body);
    builder.create<func::CallOp>(loc,
                                 builder.getStringAttr("trace_loop_iter_begin"),
                                 TypeRange{}, ValueRange{});

    builder.setInsertionPoint(body.getTerminator());
    builder.create<func::CallOp>(loc,
                                 builder.getStringAttr("trace_loop_iter_end"),
                                 TypeRange{}, ValueRange{});
  }

  void processAffineFor(affine::AffineForOp op, OpBuilder &builder) {
    insertTraceCalls(*op.getBody(), builder, op.getLoc());
  }

  void processSCFFor(scf::ForOp op, OpBuilder &builder) {
    insertTraceCalls(*op.getBody(), builder, op.getLoc());
  }

  void processSCFWhile(scf::WhileOp op, OpBuilder &builder) {
    Block &afterBlock = op.getAfter().front();
    insertTraceCalls(afterBlock, builder, op.getLoc());
  }

  void ensureTraceFunctionsExist(ModuleOp moduleOp, OpBuilder &builder) {
    auto insertFuncIfMissing = [&](StringRef name) {
      if (!moduleOp.lookupSymbol<func::FuncOp>(name)) {
        auto funcType = builder.getFunctionType({}, {});
        OpBuilder::InsertionGuard guard(builder);
        builder.setInsertionPointToStart(moduleOp.getBody());
        builder.create<func::FuncOp>(moduleOp.getLoc(), name, funcType)
            .setPrivate();
      }
    };

    insertFuncIfMissing("trace_loop_iter_end");
    insertFuncIfMissing("trace_loop_iter_begin");
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp.getContext());

    ensureTraceFunctionsExist(moduleOp, builder);

    moduleOp.walk([&](Operation *op) {
      if (auto affineFor = dyn_cast<affine::AffineForOp>(op)) {
        processAffineFor(affineFor, builder);
      } else if (auto scfFor = dyn_cast<scf::ForOp>(op)) {
        processSCFFor(scfFor, builder);
      } else if (auto scfWhile = dyn_cast<scf::WhileOp>(op)) {
        processSCFWhile(scfWhile, builder);
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TraceLoopIterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TraceLoopIterPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopPassBeginEnd_Chizhov_Maxim_FIIT3_MLIR",
          "1.0", []() { mlir::PassRegistration<TraceLoopIterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
