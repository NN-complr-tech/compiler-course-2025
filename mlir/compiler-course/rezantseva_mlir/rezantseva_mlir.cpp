#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include <algorithm>
#include <vector>

using namespace mlir;

namespace {

class MaxDepthPass
    : public PassWrapper<MaxDepthPass, OperationPass<func::FuncOp>> {
private:
  bool isLoopOp(Operation *op) {
    return isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ForallOp,
               spirv::LoopOp>(op);
  }

  bool isNestingOp(Operation *op) {
    return isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ForallOp,
               spirv::LoopOp, affine::AffineIfOp, scf::IfOp,
               spirv::SelectionOp>(op);
  }

  unsigned computeDepth(Operation *op, unsigned currentDepth = 0) {
    if (isNestingOp(op))
      currentDepth++;
    unsigned maxDepth = currentDepth;

    for (Region &region : op->getRegions()) {
      for (Block &block : region) {
        for (Operation &nestedOp : block) {
          maxDepth = std::max(maxDepth, computeDepth(&nestedOp, currentDepth));
        }
      }
    }

    return maxDepth;
  }

public:
  StringRef getArgument() const final {
    return "MaxDepthPass_RezantsevaAnastasia_FIIT1_MLIR";
  }

  StringRef getDescription() const final {
    return "The pass counts the max depth of control flow operations nests in "
           "each loop.";
  }

  void runOnOperation() override {
    func::FuncOp func = getOperation();
    std::vector<unsigned> loopDepths;

    func.walk<mlir::WalkOrder::PreOrder>([&](Operation *op) {
      if (isLoopOp(op)) {
        if (unsigned depth = computeDepth(op)) {
          loopDepths.push_back(depth);
        }
        return WalkResult::skip();
      }
      return WalkResult::advance();
    });

    if (loopDepths.empty()) {
      func->setAttr("my_loop_depths", ArrayAttr::get(func.getContext(), {}));
    } else {
      SmallVector<Attribute> depthAttrs;
      for (unsigned depth : loopDepths) {
        depthAttrs.push_back(
            IntegerAttr::get(IntegerType::get(func.getContext(), 64), depth));
      }
      func->setAttr("my_loop_depths",
                    ArrayAttr::get(func.getContext(), depthAttrs));
    }
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(MaxDepthPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(MaxDepthPass)

mlir::PassPluginLibraryInfo getMaxDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MaxDepthPass", "1.0",
          []() { mlir::PassRegistration<MaxDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMaxDepthPassPluginInfo();
}
