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

class DepthAnalyzer {
public:
  int computeStructureDepth(mlir::Operation *targetOp) {
    if (!targetOp)
      return 0;

    int maxChildDepth = 0;
    for (auto &region : targetOp->getRegions()) {
      maxChildDepth = std::max(maxChildDepth, analyzeNestedDepth(region));
    }

    return 1 + maxChildDepth;
  }

  bool isLoopOrConditionOp(mlir::Operation &op) {
    return llvm::isa<mlir::scf::ForOp, mlir::scf::WhileOp, mlir::scf::IfOp,
                     mlir::affine::AffineForOp, mlir::affine::AffineIfOp>(op);
  }

private:
  int analyzeNestedDepth(mlir::Region &region) {
    int deepestLevel = 0;

    for (auto &block : region) {
      for (auto &operation : block) {
        deepestLevel =
            std::max(deepestLevel, evaluateOperationDepth(operation));
      }
    }
    return deepestLevel;
  }

  int evaluateOperationDepth(mlir::Operation &op) {
    if (isLoopOrConditionOp(op)) {
      int maxSubDepth = 0;

      for (auto &nestedRegion : op.getRegions()) {
        maxSubDepth = std::max(maxSubDepth, analyzeNestedDepth(nestedRegion));
      }

      return 1 + maxSubDepth;
    }

    return 0;
  }
};

class ExamplePass_VasenkovAndrey_FIIT1_MLIR
    : public mlir::PassWrapper<ExamplePass_VasenkovAndrey_FIIT1_MLIR,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      ExamplePass_VasenkovAndrey_FIIT1_MLIR)

  mlir::StringRef getArgument() const final {
    return "ExamplePass_VasenkovAndrey_FIIT1_MLIR";
  }

  mlir::StringRef getDescription() const final {
    return "Analyzes loop nesting depth in functions";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    DepthAnalyzer analyzer;

    module.walk([&](mlir::func::FuncOp function) {
      processFunction(function, analyzer);
    });
  }

private:
  void processFunction(mlir::func::FuncOp function, DepthAnalyzer &analyzer) {
    llvm::SmallVector<int64_t, 4> depthResults;

    mlir::Block &entryBlock = function.getBody().front();

    for (mlir::Operation &op : entryBlock) {
      if (analyzer.isLoopOrConditionOp(op)) {
        int nestingLevel = analyzer.computeStructureDepth(&op);
        depthResults.push_back(nestingLevel);
      }
    }

    if (!depthResults.empty()) {
      mlir::OpBuilder builder(function.getContext());
      auto depthAttribute = builder.getI64ArrayAttr(depthResults);
      function->setAttr("my_loop_depths", depthAttribute);
    }
  }
};

} // namespace

static mlir::PassPluginLibraryInfo getExamplePassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "ExamplePass", LLVM_VERSION_STRING, []() {
            mlir::PassRegistration<ExamplePass_VasenkovAndrey_FIIT1_MLIR>();
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getExamplePassPluginInfo();
}