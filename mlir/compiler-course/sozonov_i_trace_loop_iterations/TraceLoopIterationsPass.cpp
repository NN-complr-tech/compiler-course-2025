#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class TraceLoopIterationsPass
    : public PassWrapper<TraceLoopIterationsPass, OperationPass<ModuleOp>> {
public:
  StringRef getArgument() const final {
    return "TraceLoopIterationsPass_Sozonov_Ilya_FIIT3_MLIR";
  }
  StringRef getDescription() const final {
    return "Insert trace calls at beginning and end of each loop iteration";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);

    auto ensureFuncDecl = [&](StringRef name) {
      if (!moduleOp.lookupSymbol<func::FuncOp>(name)) {
        auto funcType = FunctionType::get(moduleOp.getContext(), {}, {});
        OpBuilder::InsertionGuard guard(builder);
        builder.setInsertionPointToStart(moduleOp.getBody());
        builder.create<func::FuncOp>(moduleOp.getLoc(), name, funcType)
            .setPrivate();
      }
    };

    ensureFuncDecl("trace_loop_iter_begin");
    ensureFuncDecl("trace_loop_iter_end");

    moduleOp.walk([&](Operation *op) {
      if (isa<scf::ForOp, scf::WhileOp, affine::AffineForOp>(op)) {
        auto loc = op->getLoc();
        OpBuilder builder(op->getContext());

        Region &bodyRegion = op->getRegion(0);
        if (bodyRegion.empty())
          return;

        Block &entryBlock = bodyRegion.front();
        builder.setInsertionPointToStart(&entryBlock);
        builder.create<func::CallOp>(loc, "trace_loop_iter_begin",
                                     TypeRange(), ValueRange());

        for (Block &block : bodyRegion) {
          Operation *terminator = block.getTerminator();
          builder.setInsertionPoint(terminator);
          builder.create<func::CallOp>(loc, "trace_loop_iter_end",
                                       TypeRange(), ValueRange());
        }
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TraceLoopIterationsPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TraceLoopIterationsPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "TraceLoopIterationsPass", "1.0",
          []() { mlir::PassRegistration<TraceLoopIterationsPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
