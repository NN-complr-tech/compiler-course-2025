#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
// Pass insertion of trace_loop_iter_begin/end calls around loops
struct TraceLoopPass
    : public PassWrapper<TraceLoopPass, OperationPass<ModuleOp>> {
  StringRef getArgument() const final {
    return "TraceLoopPass_KudryashovaIrina_FIIT3_MLIR";
  }
  StringRef getDescription() const final {
    return "Insert @trace_loop_iter_begin and @trace_loop_iter_end calls at "
           "loop iteration boundaries";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry
        .insert<affine::AffineDialect, scf::SCFDialect, func::FuncDialect>();
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module.getContext());
    Location loc = module.getLoc();

    // Declare trace functions if not present
    if (!module.lookupSymbol<func::FuncOp>("trace_loop_iter_begin")) {
      builder.setInsertionPointToStart(module.getBody());
      auto begin = builder.create<func::FuncOp>(
          loc, "trace_loop_iter_begin", builder.getFunctionType({}, {}));
      begin.setSymVisibility("private");
    }
    if (!module.lookupSymbol<func::FuncOp>("trace_loop_iter_end")) {
      builder.setInsertionPointToStart(module.getBody());
      auto end = builder.create<func::FuncOp>(loc, "trace_loop_iter_end",
                                              builder.getFunctionType({}, {}));
      end.setSymVisibility("private");
    }

    module.walk([&](Operation *op) {
      OpBuilder b(op);
      Location loopLoc = op->getLoc();

      // affine.for: getBody() returns Block*
      if (auto forOp = dyn_cast<affine::AffineForOp>(op)) {
        Block &body = *forOp.getBody();
        b.setInsertionPointToStart(&body);
        b.create<func::CallOp>(loopLoc, "trace_loop_iter_begin", TypeRange{},
                               ValueRange{});
        Operation *term = body.getTerminator();
        OpBuilder endBuilder(term->getContext());
        endBuilder.setInsertionPoint(term);
        endBuilder.create<func::CallOp>(loopLoc, "trace_loop_iter_end",
                                        TypeRange{}, ValueRange{});
      }
      // scf.for: getBody() returns Region&, use front() to get Block&
      else if (auto scfFor = dyn_cast<scf::ForOp>(op)) {
        Block &body = *scfFor.getBody();
        b.setInsertionPointToStart(&body);
        b.create<func::CallOp>(loopLoc, "trace_loop_iter_begin", TypeRange{},
                               ValueRange{});
        Operation *term = body.getTerminator();
        OpBuilder endBuilder(term->getContext());
        endBuilder.setInsertionPoint(term);
        endBuilder.create<func::CallOp>(loopLoc, "trace_loop_iter_end",
                                        TypeRange{}, ValueRange{});
      }
      // scf.while: after region holds the body
      else if (auto whileOp = dyn_cast<scf::WhileOp>(op)) {
        Block &body = whileOp.getAfter().front();
        b.setInsertionPointToStart(&body);
        b.create<func::CallOp>(loopLoc, "trace_loop_iter_begin", TypeRange{},
                               ValueRange{});
        Operation *term = body.getTerminator();
        OpBuilder endBuilder(term->getContext());
        endBuilder.setInsertionPoint(term);
        endBuilder.create<func::CallOp>(loopLoc, "trace_loop_iter_end",
                                        TypeRange{}, ValueRange{});
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TraceLoopPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TraceLoopPass)

mlir::PassPluginLibraryInfo getTraceLoopPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TraceLoopPass", "1.0",
          []() { mlir::PassRegistration<TraceLoopPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getTraceLoopPassPluginInfo();
}
