#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

int computeMaxNestingInRegion(mlir::Region &region) {
  int maxDepth = 0;

  for (mlir::Block &block : region) {
    for (mlir::Operation &op : block.getOperations()) {
      int depthForOp = 0;

      if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                    mlir::affine::AffineForOp, mlir::scf::IfOp,
                    mlir::affine::AffineIfOp>(&op)) {
        depthForOp = 1;
        for (mlir::Region &subRegion : op.getRegions()) {
          depthForOp =
              std::max(depthForOp, 1 + computeMaxNestingInRegion(subRegion));
        }
      }

      maxDepth = std::max(maxDepth, depthForOp);
    }
  }
  return maxDepth;
}

class RegionNestingAnalysisPass
    : public mlir::PassWrapper<RegionNestingAnalysisPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const override { return "my-loop-depth-pass"; }
  mlir::StringRef getDescription() const override {
    return "Computes maximum nesting depth of regions inside loops";
  }

  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(RegionNestingAnalysisPass)

  void runOnOperation() override {
    mlir::ModuleOp moduleOp = this->getOperation();

    moduleOp.walk([&](mlir::func::FuncOp funcOp) {
      mlir::Builder builder(funcOp.getContext());
      llvm::SmallVector<int64_t, 4> loopDepths;
      funcOp.walk([&](mlir::Operation *opInFunction) {
        if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                      mlir::affine::AffineForOp>(opInFunction)) {
          int depth = 1;
          for (mlir::Region &loopRegion : opInFunction->getRegions()) {
            depth = std::max(depth, 1 + computeMaxNestingInRegion(loopRegion));
          }
          loopDepths.push_back(depth);
        }
      });

      if (!loopDepths.empty()) {
        mlir::ArrayAttr depthsAttr = builder.getI64ArrayAttr(loopDepths);
        funcOp->setAttr("my_loop_depths", depthsAttr);

        llvm::outs() << "Function '" << funcOp.getName()
                     << "': processed. Loop depths: ";
        llvm::interleaveComma(loopDepths, llvm::outs());
        llvm::outs() << "\n";
      } else {
        llvm::outs() << "Function '" << funcOp.getName()
                     << "': no tracked loops found\n";
      }
    });
  }
};

} // namespace

static mlir::PassPluginLibraryInfo getMyLoopDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MyLoopDepthPlugin", "1.0",
          []() { mlir::PassRegistration<RegionNestingAnalysisPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMyLoopDepthPassPluginInfo();
}