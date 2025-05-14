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

int calculateDepthInsideRegion(mlir::Region &InputRegion) {
  int MaxDepthFound = 0;

  for (mlir::Block &BlockElement : InputRegion) {
    for (mlir::Operation &Op : BlockElement.getOperations()) {
      int CurrentOpNestingLevel = 0;

      if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                    mlir::affine::AffineForOp, mlir::scf::IfOp,
                    mlir::affine::AffineIfOp>(&Op)) {
        CurrentOpNestingLevel = 1;

        int MaxDepthInOpSubRegions = 0;
        for (mlir::Region &SubRegion : Op.getRegions()) {
          int DepthInThisSubRegion = calculateDepthInsideRegion(SubRegion);
          if (DepthInThisSubRegion > MaxDepthInOpSubRegions) {
            MaxDepthInOpSubRegions = DepthInThisSubRegion;
          }
        }
        CurrentOpNestingLevel += MaxDepthInOpSubRegions;
      }

      if (CurrentOpNestingLevel > MaxDepthFound) {
        MaxDepthFound = CurrentOpNestingLevel;
      }
    }
  }
  return MaxDepthFound;
}

class MyLoopDepthPass
    : public mlir::PassWrapper<MyLoopDepthPass,
                               mlir::OperationPass<mlir::ModuleOp>> {
public:
  mlir::StringRef getArgument() const override { return "my-loop-depth-pass"; }
  mlir::StringRef getDescription() const override {
    return "Считает максимальную глубину вложенности регионов";
  }

  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(MyLoopDepthPass)

  void runOnOperation() override {
    mlir::ModuleOp moduleOp = this->getOperation();

    moduleOp.walk([&](mlir::func::FuncOp funcOp) {
      mlir::OpBuilder Builder(funcOp.getContext());
      llvm::SmallVector<int64_t, 4> DepthsOfAllLoopsInFunction;
      funcOp.walk([&](mlir::Operation *OpInFunction) {
        if (mlir::isa<mlir::scf::ForOp, mlir::scf::WhileOp,
                      mlir::affine::AffineForOp>(OpInFunction)) {
          int TotalDepthForThisLoop = 1;
          int MaxNestingInsideLoopBody = 0;

          for (mlir::Region &LoopRegion : OpInFunction->getRegions()) {
            int DepthInThisRegionOfLoop =
                calculateDepthInsideRegion(LoopRegion);
            if (DepthInThisRegionOfLoop > MaxNestingInsideLoopBody) {
              MaxNestingInsideLoopBody = DepthInThisRegionOfLoop;
            }
          }
          TotalDepthForThisLoop += MaxNestingInsideLoopBody;
          DepthsOfAllLoopsInFunction.push_back(TotalDepthForThisLoop);
        }
      });

      if (!DepthsOfAllLoopsInFunction.empty()) {
        mlir::ArrayAttr DepthsAttribute =
            Builder.getI64ArrayAttr(DepthsOfAllLoopsInFunction);
        funcOp->setAttr("my_loop_depths", DepthsAttribute);

        llvm::outs() << "Функция '" << funcOp.getName()
                     << "': обработана. Найденные глубины циклов: ";
        llvm::interleaveComma(DepthsOfAllLoopsInFunction, llvm::outs());
        llvm::outs() << "\n";
      } else {
        llvm::outs() << "Функция '" << funcOp.getName()
                     << "': не содержит отслеживаемых циклов\n";
      }
    });
  }
};

} // namespace

static mlir::PassPluginLibraryInfo getMyLoopDepthPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "MyLoopDepthPlugin", "1.0",
          []() { mlir::PassRegistration<MyLoopDepthPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getMyLoopDepthPassPluginInfo();
}