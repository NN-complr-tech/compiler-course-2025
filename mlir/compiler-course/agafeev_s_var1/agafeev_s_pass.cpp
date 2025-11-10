#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/TypeSwitch.h"
using namespace mlir;

namespace {

class TraceLoopPass
    : public PassWrapper<TraceLoopPass, OperationPass<ModuleOp>> {
  template <typename LoopOperation>
  void processLoop(LoopOperation operation, Block &body, ValueRange indVars,
                   OpBuilder &builder) {
    Location loc = operation.getLoc();

    std::string beginName =
        "trace_loop_iter_begin_" + std::to_string(indVars.size());
    std::string endName =
        "trace_loop_iter_end_" + std::to_string(indVars.size());

    builder.setInsertionPointToStart(&body);
    builder.create<func::CallOp>(loc, beginName, TypeRange{}, indVars);

    builder.setInsertionPoint(body.getTerminator());
    builder.create<func::CallOp>(loc, endName, TypeRange{}, indVars);
  }

  template <typename OpTy>
  void processFor(OpTy op, OpBuilder &builder) {
    processLoop(op, *op.getBody(), op.getInductionVar(), builder);
  }

  void processWhileOp(scf::WhileOp op, OpBuilder &builder) {
    Block &body = op.getAfter().front();
    auto beforeArgs = op.getBeforeArguments();
    auto afterArgs = op.getAfterArguments();

    Location loc = op.getLoc();
    std::string beginName =
        "trace_loop_iter_begin_" + std::to_string(beforeArgs.size());
    std::string endName =
        "trace_loop_iter_end_" + std::to_string(beforeArgs.size());

    builder.setInsertionPointToStart(&body);
    builder.create<func::CallOp>(loc, beginName, TypeRange{}, afterArgs);

    builder.setInsertionPoint(body.getTerminator());
    builder.create<func::CallOp>(loc, endName, TypeRange{}, afterArgs);
  }

  template <typename OpTy>
  void processWhile(OpTy op, OpBuilder &builder) {
    processWhileOp(op, builder);
  }

public:
  StringRef getArgument() const final {
    return "MlirLoopPass_Agafeev_Sergey_FIIT3_MLIR";
  }

  StringRef getDescription() const final {
    return "Insert trace calls at beginning and end of each loop iteration";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();
    OpBuilder builder(moduleOp);
    moduleOp.walk([&](Operation *op) {
      TypeSwitch<Operation *>(op)
          .Case<affine::AffineForOp>([&](auto op) { processFor(op, builder); })
          .Case<affine::AffineParallelOp>(
              [&](auto op) { processFor(op, builder); })
          .Case<scf::ForOp>([&](auto op) { processFor(op, builder); })
          .Case<scf::WhileOp>([&](auto op) { processWhile(op, builder); })
          .Default([](auto) {});
    });
  }
};

template <>
void TraceLoopPass::processFor(affine::AffineParallelOp op,
                               OpBuilder &builder) {
  processLoop(op, *op.getBody(), op.getIVs(), builder);
}
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(TraceLoopPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(TraceLoopPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MlirLoopPass_Agafeev_Sergey_FIIT3_MLIR",
          "1.0", []() { mlir::PassRegistration<TraceLoopPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
