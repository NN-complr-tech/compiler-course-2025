// Write a pass that counts the max depth of region nests in each loop (loops:
// affine.for, scf.for, scf.while, ...). E.g. normal loop has depth 1, loop with
// another loop or if stmt inside has depth 2, loop {if {if {}}} has depth 3 and
// so on. Attach the result as an attribute for the function (func.func)
// operation

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include <stack>
#include <vector>

using namespace mlir;

namespace {

class LoopNestsPass
    : public PassWrapper<LoopNestsPass, OperationPass<func::FuncOp>> {
protected:
  class myStack {
  public:
    Operation *operation;
    size_t currentDepth;
  };
  std::vector<size_t> maxDepths;

public:
  StringRef getArgument() const final {
    return "LoopNestsPass_KabalovaValeria_FIIT1_MLIR";
  }
  StringRef getDescription() const final {
    return "It's a pass, that counts the max depth of region nests in each "
           "loop. It supports "
           "loops such as: affine.for, scf.for, scf.while, scf.forall, "
           "spirv.mlir.loop. Pass also counts depth with if-statement inside "
           "the loop. "
           "It supports if-statements such as: affine.if, scf.if, "
           "spirv.mlir.selection. ";
  }

  void runOnOperation() override {
    func::FuncOp func = getOperation();

    for (Operation &op : func.getBody().front()) {
      if (isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ForallOp,
              spirv::LoopOp>(op)) {
        size_t maxDepth = getMaxDepth(&op);
        maxDepths.push_back(maxDepth);
      } else if (isa<spirv::LoopOp, affine::AffineIfOp, scf::IfOp,
                     spirv::SelectionOp>(op)) {
        findLoops(&op);
      }
    }

    SmallVector<Attribute> depthAttributes;
    for (size_t i : maxDepths) {
      depthAttributes.push_back(
          IntegerAttr::get(IntegerType::get(func.getContext(), 64), i));
    }
    maxDepths.clear();
    func->setAttr("Max_loop_depths:",
                  ArrayAttr::get(func.getContext(), depthAttributes));
  }

  void findLoops(Operation *root) {
    for (Region &region : root->getRegions()) {
      for (Block &block : region) {
        for (Operation &op : block) {
          if (isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ForallOp,
                  spirv::LoopOp>(op)) {
            size_t maxDepth = getMaxDepth(&op);
            maxDepths.push_back(maxDepth);
          } else if (isa<affine::AffineIfOp, scf::IfOp, spirv::SelectionOp>(
                         op)) {
            findLoops(&op);
          }
        }
      }
    }
  }

  size_t getMaxDepth(Operation *root) {
    std::stack<myStack> stack;
    stack.push({root, 0});
    size_t maxDepth = 0;
    while (!stack.empty()) {
      myStack element = stack.top();
      stack.pop();
      Operation *currentOp = element.operation;
      size_t currentDepth = element.currentDepth;

      if (isa<affine::AffineForOp, scf::ForOp, scf::WhileOp, scf::ForallOp,
              spirv::LoopOp, affine::AffineIfOp, scf::IfOp, spirv::SelectionOp>(
              currentOp)) {
        currentDepth += 1;
        maxDepth = std::max(maxDepth, currentDepth);
      }

      for (Region &region : currentOp->getRegions()) {
        for (Block &block : region) {
          for (Operation &operation : block) {
            stack.push({&operation, currentDepth});
          }
        }
      }
    }
    return maxDepth;
  }
};

} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopNestsPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopNestsPass)

mlir::PassPluginLibraryInfo getLoopNestsPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopNestsPass", "1.0",
          []() { mlir::PassRegistration<LoopNestsPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLoopNestsPluginInfo();
}
