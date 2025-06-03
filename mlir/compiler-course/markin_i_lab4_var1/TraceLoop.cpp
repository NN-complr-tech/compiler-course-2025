#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

namespace {

struct LoopTracerPass
    : public mlir::PassWrapper<LoopTracerPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
  void addTraceMarkers(mlir::Block &block, mlir::Location loc,
                       mlir::OpBuilder &builder) {
    builder.setInsertionPointToStart(&block);
    builder.create<mlir::func::CallOp>(loc, "loop_begin_marker",
                                       mlir::TypeRange(), mlir::ValueRange());
    if (mlir::Operation *terminator = block.getTerminator()) {
      builder.setInsertionPoint(terminator);
      builder.create<mlir::func::CallOp>(loc, "loop_end_marker",
                                         mlir::TypeRange(), mlir::ValueRange());
    }
  }

public:
  mlir::StringRef getArgument() const final {
    return "TraceLoopPass_MarkinIvan_FIIT2_MLIR";
  }

  mlir::StringRef getDescription() const final {
    return "Inserts loop tracing markers.";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::MLIRContext *context = module.getContext();
    mlir::OpBuilder builder(context);
    mlir::SymbolTable symbolTable(module);
    if (!symbolTable.lookup("loop_begin_marker")) {
      auto funcType = mlir::FunctionType::get(context, {}, {});
      auto newFunc = mlir::func::FuncOp::create(module.getLoc(),
                                                "loop_begin_marker", funcType);
      newFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
      symbolTable.insert(newFunc);
    }
    if (!symbolTable.lookup("loop_end_marker")) {
      auto funcType = mlir::FunctionType::get(context, {}, {});
      auto newFunc = mlir::func::FuncOp::create(module.getLoc(),
                                                "loop_end_marker", funcType);
      newFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
      symbolTable.insert(newFunc);
    }

    module.walk([&](mlir::Operation *op) {
      if (auto affineForOp = mlir::dyn_cast<mlir::affine::AffineForOp>(op)) {
        addTraceMarkers(*affineForOp.getBody(), affineForOp->getLoc(), builder);
      } else if (auto forOp = mlir::dyn_cast<mlir::scf::ForOp>(op)) {
        addTraceMarkers(*forOp.getBody(), forOp->getLoc(), builder);
      } else if (auto whileOp = mlir::dyn_cast<mlir::scf::WhileOp>(op)) {
        addTraceMarkers(whileOp.getAfter().front(), whileOp->getLoc(), builder);
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopTracerPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopTracerPass)

mlir::PassPluginLibraryInfo getLoopTracerPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopTracerPass", "1",
          []() { mlir::PassRegistration<LoopTracerPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLoopTracerPassPluginInfo();
}