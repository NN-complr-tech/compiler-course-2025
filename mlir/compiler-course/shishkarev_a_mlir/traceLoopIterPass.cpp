#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

using namespace mlir;

namespace {
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

private:
  void createTraceFunction(ModuleOp module, OpBuilder &builder, 
                          StringRef functionName) {
    if (!module.lookupSymbol<func::FuncOp>(functionName)) {
      Location loc = module.getLoc();
      builder.setInsertionPointToStart(module.getBody());
      auto func = builder.create<func::FuncOp>(
          loc, functionName, builder.getFunctionType({}, {}));
      func.setSymVisibility("private");
    }
  }

  void instrumentLoopBody(OpBuilder &builder, Block &body, Location loopLoc) {
    builder.setInsertionPointToStart(&body);
    builder.create<func::CallOp>(loopLoc, "trace_loop_iter_begin",
                                 TypeRange{}, ValueRange{});
    
    Operation *term = body.getTerminator();
    builder.setInsertionPoint(term);
    builder.create<func::CallOp>(loopLoc, "trace_loop_iter_end",
                                 TypeRange{}, ValueRange{});
  }

public:
  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module.getContext());

    createTraceFunction(module, builder, "trace_loop_iter_begin");
    createTraceFunction(module, builder, "trace_loop_iter_end");

    module.walk([&](Operation *op) {
      OpBuilder::InsertionGuard guard(builder);
      
      if (auto forOp = dyn_cast<affine::AffineForOp>(op)) {
        instrumentLoopBody(builder, *forOp.getBody(), op->getLoc());
      } else if (auto scfFor = dyn_cast<scf::ForOp>(op)) {
        instrumentLoopBody(builder, *scfFor.getBody(), op->getLoc());
      } else if (auto whileOp = dyn_cast<scf::WhileOp>(op)) {
        instrumentLoopBody(builder, whileOp.getAfter().front(), op->getLoc());
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