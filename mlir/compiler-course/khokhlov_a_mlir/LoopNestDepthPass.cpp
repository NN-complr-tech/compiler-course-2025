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

int computeRegionNestingDepth(mlir::Region &Region) {
  int MaxDepth = 0;

  // Обходим все блоки в регионе
  for (mlir::Block &block : Region) {
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
      MaxDepth = std::max(MaxDepth, opDepth);
    }
  }
  return MaxDepth;
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
    mlir::ModuleOp Module = getOperation();
    mlir::OpBuilder Builder(Module.getContext());

    // Обходим все функции в модуле
    Module.walk([&](mlir::func::FuncOp Func) {
      llvm::SmallVector<int64_t> LoopDepths;

      // Обходим все операции в функции
      Func.walk([&](mlir::Operation *Op) {
        if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                      mlir::affine::AffineForOp>(Op)) {
          // Начальная глубина цикла = 1
          int Depth = 1;

          // Проверяем вложенные регионы
          int MaxNestedDepth = 0;
          for (mlir::Region &Region : Op->getRegions()) {
            int regionDepth = computeRegionNestingDepth(Region);
            MaxNestedDepth = std::max(MaxNestedDepth, regionDepth);
          }

          // Общая глубина = 1 + максимальная глубина вложенных регионов
          LoopDepths.push_back(Depth + MaxNestedDepth);
        }
      });

      // Устанавливаем атрибут, если найдены циклы
      if (!LoopDepths.empty()) {
        Func->setAttr("my_loop_depths", Builder.getI64ArrayAttr(LoopDepths));
        auto &os = llvm::outs();
        os << "Function '" << Func.getName()
                     << "': processed. Loop depths: ";
        llvm::interleaveComma(LoopDepths, os);
        os << "\n";
      } else {
        os << "Function '" << Func.getName() << "': no loops found\n";
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