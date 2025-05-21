#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/ADT/TypeSwitch.h"
using namespace mlir;

namespace {
class ExamplePass : public PassWrapper<ExamplePass, OperationPass<ModuleOp>> {
  template <typename LoopOp>
  void processLoop(LoopOp op, Block &body, ValueRange indVars,
                   OpBuilder &builder) {
    Location loc = op.getLoc();

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
  template <typename OpTy>
  void processWhile(OpTy op, OpBuilder &builder) {
    processWhileOp(op, builder);
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

public:
  StringRef getArgument() const final {
    return "MlirPassLoopIterBeginEnd_Baranov_Aleksey_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "Inserts a `@trace_loop_iter_begin_*` fuction call on each loop "
           "interation begin (loops: affine.for, scf.for, scf.while, ...), "
           "`@trace_loop_iter_end` fuction call at the end. The number of "
           "indVars is presented in name of the function.";
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
void ExamplePass::processFor(affine::AffineParallelOp op, OpBuilder &builder) {
  processLoop(op, *op.getBody(), op.getIVs(), builder);
}
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(ExamplePass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(ExamplePass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION,
          "MlirPassLoopIterBeginEnd_Baranov_Aleksey_FIIT1_MLIR", "1.0",
          []() { mlir::PassRegistration<ExamplePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
