#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

namespace {

int computeRegionNestingDepth(mlir::Region &region) {
  int maxDepth = 0;

  // Обходим все блоки в регионе
  for (mlir::Block &block : region) {
    for (mlir::Operation &op : block) {
      // Проверяем, является ли операция циклом или условным оператором
      if (op.hasTrait<mlir::OpTrait::IsTerminator>() ||
          !(mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                      mlir::affine::AffineForOp, mlir::scf::IfOp,
                      mlir::affine::AffineIfOp>(op))) {
        continue;
      }

      // Начальная глубина для текущей операции
      int opDepth = 1;

      // Рекурсивно вычисляем максимальную глубину вложенных регионов
      int maxSubRegionDepth = 0;
      for (mlir::Region &subRegion : op.getRegions()) {
        int subDepth = computeRegionNestingDepth(subRegion);
        maxSubRegionDepth = std::max(maxSubRegionDepth, subDepth);
      }

      // Общая глубина = 1 (текущая операция) + максимальная глубина подрегионов
      opDepth += maxSubRegionDepth;

      // Обновляем максимальную глубину
      maxDepth = std::max(maxDepth, opDepth);
    }
  }
  return maxDepth;
}

class LoopNestDepthPass
    : public mlir::PassWrapper<LoopNestDepthPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(LoopNestDepthPass)

  mlir::StringRef getArgument() const override { return "loop-nest-depth"; }
  mlir::StringRef getDescription() const override {
    return "Computes the maximum nesting depth of loops and conditionals in "
           "functions";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::OpBuilder builder(module.getContext());

    // Обходим все функции в модуле
    module.walk([&](mlir::func::FuncOp func) {
      llvm::SmallVector<int64_t> loopDepths;

      // Обходим все операции в функции
      func.walk([&](mlir::Operation *op) {
        if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                      mlir::affine::AffineForOp>(op)) {
          // Начальная глубина цикла = 1
          int depth = 1;

          // Проверяем вложенные регионы
          int maxNestedDepth = 0;
          for (mlir::Region &region : op->getRegions()) {
            int regionDepth = computeRegionNestingDepth(region);
            maxNestedDepth = std::max(maxNestedDepth, regionDepth);
          }

          // Общая глубина = 1 + максимальная глубина вложенных регионов
          loopDepths.push_back(depth + maxNestedDepth);
        }
      });

      // Устанавливаем атрибут, если найдены циклы
      if (!loopDepths.empty()) {
        func->setAttr("my_loop_depths", builder.getI64ArrayAttr(loopDepths));
        llvm::outs() << "Function '" << func.getName()
                     << "': processed. Loop depths: ";
        llvm::interleaveComma(loopDepths, llvm::outs());
        llvm::outs() << "\n";
      } else {
        llvm::outs() << "Function '" << func.getName() << "': no loops found\n";
      }
    });
  }
};

} // namespace

static mlir::PassPluginLibraryInfo getPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopNestDepthPass", "1.0.0",
          []() { mlir::PassRegistration<LoopNestDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getPassPluginInfo();
}