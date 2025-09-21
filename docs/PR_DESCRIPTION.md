# Compiler Course — Retake Pack

This PR contains 4 small labs + theoretical notes for the course *"Введение в теорию трансляторов"*.

## Состав
1. **ClangAST lab** — инструмент LibTooling, проход по AST и отчёт по конструкциям.
2. **LLVM IR lab** — плагин нового PassManager'а: заменяет `mul %x, 2` на `add %x, %x`.
3. **Backend lab** — MachineFunction-проход: удаляет тривиальные `COPY` (src == dst).
4. **MLIR lab** — Canonicalize‑проход: `arith.addi %x, 0` → `%x`.
5. **Теория** — краткий конспект по стадиям компиляции и роли LLVM/Clang/MLIR.

## Как собрать
Сборка LLVM/Clang/MLIR из корня репозитория по инструкции README (Ninja, mold, clang).
Эти модули оформлены как плагины/утилиты и собираются вместе с основной сборкой.

## Как запустить тесты
```bash
cmake --build build --config Release -t         check-llvm-compiler-course         check-clang-compiler-course         check-mlir-compiler-course -j 4
```

## Что было реализовано (кратко)
- ClangAST: обход AST с подсчётом `if`, `for`, `while`, `return`, бинарных операций.
- LLVM IR: локальная микрооптимизация с использованием PatternMatch, новый PM plugin.
- Backend: MachineFunctionPass над MIR, вычищающий бессмысленные COPY.
- MLIR: Fold `add i32, 0` → `%x` с использованием RewritePatternSet + GreedyPatternRewriteDriver.
