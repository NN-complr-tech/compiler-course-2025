#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

namespace {
class LoopTracePass
    : public mlir::PassWrapper<LoopTracePass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const final override {
    return "LoopTracePass_DrozhdinovD_FIIT1_MLIR";
  }

  mlir::StringRef getDescription() const final override {
    return "Insert @trace_loop_iter_begin and @trace_loop_iter_end function "
           "calls in the start and the end of loops";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::OpBuilder builder(module.getContext());

    Declarator("trace_loop_iter_begin", module, builder);
    Declarator("trace_loop_iter_end", module, builder);

    module.walk([&](mlir::Operation *op) {
      if (!isRecognizedLoop(op))
        return;

      for (mlir::Region &region : op->getRegions()) {
        if (region.empty())
          continue;

        mlir::Block &block = region.front();

        builder.setInsertionPointToStart(&block);
        builder.create<mlir::func::CallOp>(
            op->getLoc(), "trace_loop_iter_begin", mlir::TypeRange{},
            mlir::ValueRange{});

        builder.setInsertionPoint(block.getTerminator());
        builder.create<mlir::func::CallOp>(op->getLoc(), "trace_loop_iter_end",
                                           mlir::TypeRange{},
                                           mlir::ValueRange{});
      }
    });
  }

private:
  bool isRecognizedLoop(mlir::Operation *op) {
    return llvm::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                     mlir::scf::ParallelOp, mlir::affine::AffineForOp>(op);
  }

  void Declarator(mlir::StringRef name, mlir::ModuleOp module,
                  mlir::OpBuilder &builder) {
    if (module.lookupSymbol<mlir::func::FuncOp>(name)) {
      return;
    }
    builder.setInsertionPointToStart(module.getBody());
    auto func = builder.create<mlir::func::FuncOp>(
        module.getLoc(), name, builder.getFunctionType({}, {}));
    func.setSymVisibility("private");
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
