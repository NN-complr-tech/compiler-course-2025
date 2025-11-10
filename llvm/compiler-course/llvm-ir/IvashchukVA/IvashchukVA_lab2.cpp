#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct MarkPureFunctionsPass : public PassInfoMixin<MarkPureFunctionsPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    if (F.isDeclaration() || F.isIntrinsic())
      return PreservedAnalyses::all();

    // Пропускаем функции, которые уже помечены как readnone
    if (F.doesNotAccessMemory())
      return PreservedAnalyses::all();

    bool isPure = true;

    for (auto &BB : F) {
      for (auto &I : BB) {
        // Проверяем вызовы функций
        if (auto *Call = dyn_cast<CallBase>(&I)) {
          Function *Callee = Call->getCalledFunction();
          if (Callee && !Callee->isIntrinsic()) {
            // Если вызванная функция не readnone, то и текущая не чистая
            if (!Callee->doesNotAccessMemory()) {
              isPure = false;
              break;
            }
          } else if (!Callee) {
            // Косвенный вызов - считаем нечистым для безопасности
            isPure = false;
            break;
          }
        }

        // Проверяем операции с памятью
        if (I.mayWriteToMemory()) {
          isPure = false;
          break;
        }

        // Проверяем операции загрузки из памяти
        if (I.mayReadFromMemory()) {
          // Если есть чтение из памяти, но не из константной памяти
          // считаем нечистым
          if (!isa<LoadInst>(&I) || !cast<LoadInst>(&I)->isVolatile()) {
            // Более строгая проверка: любое чтение из памяти считаем нечистым
            // кроме чтения констант
            if (!isa<LoadInst>(&I) ||
                !isa<GlobalVariable>(cast<LoadInst>(&I)->getPointerOperand()) ||
                !cast<GlobalVariable>(cast<LoadInst>(&I)->getPointerOperand())
                     ->isConstant()) {
              isPure = false;
              break;
            }
          }
        }
      }
      if (!isPure)
        break;
    }

    if (isPure) {
      F.setDoesNotAccessMemory(true);
      errs() << "MARKED PURE: " << F.getName() << "\n";
    }

    return PreservedAnalyses::all();
  }
};

} // end anonymous namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "IvashchukVA Lab2 - Mark Pure Functions",
          LLVM_VERSION_STRING, [](PassBuilder &PB) {
            // Регистрируем pass для оптимизационного пайплайна
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "mark-pure-functions") {
                    FPM.addPass(MarkPureFunctionsPass());
                    return true;
                  }
                  return false;
                });

            // Регистрируем pass для использования в тестах
            PB.registerVectorizerStartEPCallback(
                [](FunctionPassManager &FPM, OptimizationLevel Level) {
                  FPM.addPass(MarkPureFunctionsPass());
                });
          }};
}