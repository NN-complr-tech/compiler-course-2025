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
class LoopTracingPass
    : public PassWrapper<LoopTracingPass, OperationPass<ModuleOp>> {
private:
  void instrumentLoopBody(Operation *loopOp, Block *bodyBlock,
                          ValueRange inductionVars, OpBuilder &rewriter) {
    Location opLocation = loopOp->getLoc();
    size_t varCount = inductionVars.size();

    std::string loopStartMarker =
        "trace_loop_iter_begin_" + std::to_string(varCount);
    std::string loopEndMarker =
        "trace_loop_iter_end_" + std::to_string(varCount);

    rewriter.setInsertionPointToStart(bodyBlock);
    rewriter.create<func::CallOp>(opLocation, loopStartMarker, TypeRange{},
                                  inductionVars);

    rewriter.setInsertionPoint(bodyBlock->getTerminator());
    rewriter.create<func::CallOp>(opLocation, loopEndMarker, TypeRange{},
                                  inductionVars);
  }

  void handleForLikeLoop(Operation *forOp, OpBuilder &rewriter) {
    if (auto affineFor = dyn_cast<affine::AffineForOp>(forOp)) {
      instrumentLoopBody(affineFor, affineFor.getBody(),
                         ValueRange{affineFor.getInductionVar()}, rewriter);
    } else if (auto scfFor = dyn_cast<scf::ForOp>(forOp)) {
      instrumentLoopBody(scfFor, scfFor.getBody(),
                         ValueRange{scfFor.getInductionVar()}, rewriter);
    } else if (auto affineParallel =
                   dyn_cast<affine::AffineParallelOp>(forOp)) {
      instrumentLoopBody(affineParallel, affineParallel.getBody(),
                         affineParallel.getIVs(), rewriter);
    }
  }

  void handleWhileLoop(scf::WhileOp whileOp, OpBuilder &rewriter) {
    Block &afterBlock = whileOp.getAfter().front();
    ValueRange beforeArgs = whileOp.getBeforeArguments();
    ValueRange afterArgs = whileOp.getAfterArguments();

    Location opLocation = whileOp.getLoc();
    size_t argCount = beforeArgs.size();

    std::string whileStartMarker =
        "trace_loop_iter_begin_" + std::to_string(argCount);
    std::string whileEndMarker =
        "trace_loop_iter_end_" + std::to_string(argCount);

    rewriter.setInsertionPointToStart(&afterBlock);
    rewriter.create<func::CallOp>(opLocation, whileStartMarker, TypeRange{},
                                  afterArgs);

    rewriter.setInsertionPoint(afterBlock.getTerminator());
    rewriter.create<func::CallOp>(opLocation, whileEndMarker, TypeRange{},
                                  afterArgs);
  }

public:
  StringRef getArgument() const final {
    return "mlirtraceloop_Anikin_Maksim_FIIT2_MLIR";
  }

  StringRef getDescription() const final {
    return "Instruments loop operations with tracing function calls. "
           "Inserts @trace_loop_iter_begin_* at loop entry and "
           "@trace_loop_iter_end_* before loop termination. "
           "The suffix indicates the number of induction variables.";
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder instrumentationBuilder(module);

    auto loopInstrumentation = [&](Operation *op) {
      TypeSwitch<Operation *>(op)
          .Case<affine::AffineForOp, affine::AffineParallelOp, scf::ForOp>(
              [&](auto loopOp) {
                handleForLikeLoop(loopOp, instrumentationBuilder);
              })
          .Case<scf::WhileOp>([&](scf::WhileOp whileOp) {
            handleWhileLoop(whileOp, instrumentationBuilder);
          })
          .Default([](auto) {});
    };

    module.walk(loopInstrumentation);
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(::LoopTracingPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(::LoopTracingPass)

PassPluginLibraryInfo getLoopTracingPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "mlirtraceloop_Anikin_Maksim_FIIT2_MLIR",
          LLVM_VERSION_STRING, []() { PassRegistration<LoopTracingPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo mlirGetPassPluginInfo() {
  return getLoopTracingPluginInfo();
}
