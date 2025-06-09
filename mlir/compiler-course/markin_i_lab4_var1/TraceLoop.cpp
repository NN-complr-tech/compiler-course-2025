#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"

namespace {

struct LoopTracerPass
    : public mlir::PassWrapper<LoopTracerPass,
                               mlir::OperationPass<mlir::ModuleOp>> {

  // Constants for function names
  static constexpr mlir::StringRef kLoopBeginMarker = "loop_begin_marker";
  static constexpr mlir::StringRef kLoopEndMarker = "loop_end_marker";

  void addTraceMarkers(mlir::Block &block, mlir::Location loc,
                       mlir::OpBuilder &builder,
                       mlir::SymbolTable &symbolTable) {

    // Helper function to check if a call to a specific marker exists
    auto markerCallExists = [&](mlir::StringRef markerName) {
      for (auto &op : block) {
        if (auto callOp = mlir::dyn_cast<mlir::func::CallOp>(&op)) {
          if (callOp.getCallee() == markerName) {
            return true;
          }
        }
      }
      return false;
    };

    // Check for existing loop_begin_marker
    if (!markerCallExists(kLoopBeginMarker)) {
      builder.setInsertionPointToStart(&block);
      builder.create<mlir::func::CallOp>(loc, kLoopBeginMarker,
                                         mlir::TypeRange(), mlir::ValueRange());
    }

    if (mlir::Operation *terminator = block.getTerminator()) {
      // Check for existing loop_end_marker
      if (!markerCallExists(kLoopEndMarker)) {
        builder.setInsertionPoint(terminator);
        builder.create<mlir::func::CallOp>(
            loc, kLoopEndMarker, mlir::TypeRange(), mlir::ValueRange());
      }
    }
  }

public:
  mlir::StringRef getArgument() const final {
    return "TraceLoopPass_MarkinIvan_FIIT2_MLIR";
  }

  mlir::StringRef getDescription() const final {
    return "Inserts loop tracing markers.";
  }

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::MLIRContext *context = module.getContext();
    mlir::OpBuilder builder(context);
    mlir::SymbolTable symbolTable(module);

    // Function to create and insert the marker functions
    auto createMarkerFunction = [&](mlir::StringRef funcName) {
      if (!symbolTable.lookup(funcName)) {
        auto funcType = mlir::FunctionType::get(context, {}, {});
        auto newFunc =
            mlir::func::FuncOp::create(module.getLoc(), funcName, funcType);
        newFunc.setVisibility(mlir::SymbolTable::Visibility::Private);
        symbolTable.insert(newFunc);
      }
    };

    // Create the marker functions if they don't exist
    createMarkerFunction(kLoopBeginMarker);
    createMarkerFunction(kLoopEndMarker);

    module.walk([&](mlir::Operation *op) {
      if (auto affineForOp = mlir::dyn_cast<mlir::affine::AffineForOp>(op)) {
        addTraceMarkers(*affineForOp.getBody(), affineForOp->getLoc(), builder,
                        symbolTable); // Pass symbolTable
      } else if (auto forOp = mlir::dyn_cast<mlir::scf::ForOp>(op)) {
        addTraceMarkers(*forOp.getBody(), forOp->getLoc(), builder,
                        symbolTable); // Pass symbolTable
      } else if (auto whileOp = mlir::dyn_cast<mlir::scf::WhileOp>(op)) {
        addTraceMarkers(whileOp.getAfter().front(), whileOp->getLoc(), builder,
                        symbolTable); // Pass symbolTable
      }
    });
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(LoopTracerPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(LoopTracerPass)

mlir::PassPluginLibraryInfo getLoopTracerPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "LoopTracerPass", "1",
          []() { mlir::PassRegistration<LoopTracerPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getLoopTracerPassPluginInfo();
}
