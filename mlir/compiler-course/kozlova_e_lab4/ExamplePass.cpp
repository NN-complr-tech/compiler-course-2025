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

int computeRegionDepth(mlir::Region &region) {
  int maxDepth = 0;

  for (mlir::Block &block : region) {
    for (mlir::Operation &op : block.getOperations()) {
      int currentDepth = 0;

      if (llvm::isa<mlir::scf::ForOp, mlir::scf::WhileOp, mlir::scf::IfOp,
                    mlir::affine::AffineForOp, mlir::affine::AffineIfOp>(op)) {
        currentDepth = 1;

        int nestedDepth = 0;
        for (mlir::Region &subRegion : op.getRegions()) {
          nestedDepth = std::max(nestedDepth, computeRegionDepth(subRegion));
        }

        currentDepth += nestedDepth;
      }

      maxDepth = std::max(maxDepth, currentDepth);
    }
  }

  return maxDepth;
}

int getLoopDepth(mlir::Operation *loopOp) {
  int nestedDepth = 0;
  for (mlir::Region &region : loopOp->getRegions()) {
    nestedDepth = std::max(nestedDepth, computeRegionDepth(region));
  }
  return 1 + nestedDepth;
}

class ExamplePass
    : public mlir::PassWrapper<ExamplePass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(ExamplePass)

  mlir::StringRef getArgument() const override {
    return "ExamplePass_Kozlova_Ekaterina_FIIT3_MLIR";
  }

  mlir::StringRef getDescription() const override {
    return "Calculates the maximum nesting depth of regions";
  }

  void runOnOperation() override {
    mlir::ModuleOp moduleOp = getOperation();

    moduleOp.walk([&](mlir::func::FuncOp funcOp) {
      llvm::SmallVector<int64_t, 4> loopDepths;
      mlir::Block &entryBlock = funcOp.getBody().front();
      for (mlir::Operation &op : entryBlock) {
        if (llvm::isa<mlir::scf::ForOp, mlir::scf::WhileOp, mlir::scf::IfOp,
                      mlir::affine::AffineForOp, mlir::affine::AffineIfOp>(
                op)) {
          int depth = getLoopDepth(&op);
          loopDepths.push_back(depth);
        }
      };

      if (!loopDepths.empty()) {
        mlir::OpBuilder builder(funcOp.getContext());
        auto depthAttr = builder.getI64ArrayAttr(loopDepths);
        funcOp->setAttr("my_loop_depths", depthAttr);
      }
    });
  }
};

} // namespace

static mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ExamplePass", "1.0",
          []() { mlir::PassRegistration<ExamplePass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
