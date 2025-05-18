#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

namespace {

struct TraceLoopPass
    : public mlir::PassWrapper<TraceLoopPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
  void insertTraceLoop(mlir::Block &blockBefore, mlir::Block &blockAfter,
                       mlir::Location loc, mlir::OpBuilder &opBuilder) {
    opBuilder.setInsertionPointToStart(&blockBefore);
    opBuilder.create<mlir::func::CallOp>(loc, "trace_loop_iter_begin",
                                         mlir::TypeRange(), mlir::ValueRange());
    for (auto &op : blockAfter) {
      if (mlir::isa<mlir::affine::AffineYieldOp>(op) ||
          mlir::isa<mlir::scf::YieldOp>(op)) {
        opBuilder.setInsertionPoint(&op);
        opBuilder.create<mlir::func::CallOp>(
            loc, "trace_loop_iter_end", mlir::TypeRange(), mlir::ValueRange());
      }
    }
  }

public:
  mlir::StringRef getArgument() const final {
    return "TraceLoopPass_Kurakin_Matvey_FIIT1_MLIR";
  }

  mlir::StringRef getDescription() const final { return "Trace Loop Pass"; }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::MLIRContext *context = module.getContext();
    mlir::OpBuilder opBuilder(context);

    mlir::SymbolTable symbolTable(module);
    if (!symbolTable.lookup("trace_loop_iter_begin")) {
      auto funcType = mlir::FunctionType::get(context, {}, {});
      auto newFunc = mlir::func::FuncOp::create(
          module.getLoc(), "trace_loop_iter_begin", funcType);
      newFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
      symbolTable.insert(newFunc);
    }
    if (!symbolTable.lookup("trace_loop_iter_end")) {
      auto funcType = mlir::FunctionType::get(context, {}, {});
      auto newFunc = mlir::func::FuncOp::create(
          module.getLoc(), "trace_loop_iter_end", funcType);
      newFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
      symbolTable.insert(newFunc);
    }

    module.walk([&](mlir::Operation *op) {
      if (auto affineForOp = mlir::dyn_cast<mlir::affine::AffineForOp>(op)) {
        insertTraceLoop(*affineForOp.getBody(), *affineForOp.getBody(),
                        affineForOp->getLoc(), opBuilder);
      } else if (auto forOp = mlir::dyn_cast<mlir::scf::ForOp>(op)) {
        insertTraceLoop(*forOp.getBody(), *forOp.getBody(), forOp->getLoc(),
                        opBuilder);
      } else if (auto whileOp = mlir::dyn_cast<mlir::scf::WhileOp>(op)) {
        insertTraceLoop(whileOp.getBefore().front(), whileOp.getAfter().front(),
                        whileOp->getLoc(), opBuilder);
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TraceLoopPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TraceLoopPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TraceLoopPass", "1.0",
          []() { mlir::PassRegistration<TraceLoopPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
