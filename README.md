# Compiler course 2025

[![Build LLVM](https://github.com/NN-complr-tech/compiler-course-2025/actions/workflows/compiler-course-build.yml/badge.svg)](https://github.com/NN-complr-tech/compiler-course-2025/actions/workflows/compiler-course-build.yml)

# Resources
- [Telegram сhat][chat]
- [Telegram сhannel][channel]
- [Tasks and results][results]
- Materials
    - [Lecture recordings][recording]
    - [Lecture presentations][lecture]
    - [LLVM][llvm]
    - [MLIR][mlir]
    - [Clang][clang]
    - [FileCheck][filecheck]
    - [Official YouTube channel LLVM][youtube_llvm]

<!-- LINKS -->
<!-- Tasks and results -->
[results]: https://docs.google.com/spreadsheets/d/1LiZ5FMd5t61yoGdnpANTFpzqtKD_ejtvLl1cHKZxvXQ/edit?usp=sharing
<!-- Contacts -->
[channel]: https://t.me/+TPntKPD8z0E3OWJi
[chat]: https://t.me/+JG3n1jeSAiIxZjMy
<!-- Materials -->
[recording]: https://disk.yandex.ru/d/52gu5vJTSt1VFg
[lecture]: https://github.com/NN-complr-tech/Complr-course-lectures
[llvm]: https://llvm.org/
[mlir]: https://mlir.llvm.org/
[clang]: https://clang.llvm.org/
[filecheck]: https://llvm.org/docs/CommandGuide/FileCheck.html
[youtube_llvm]: https://www.youtube.com/@LLVMPROJ

# What is LLVM?
LLLVM is a set of compiler and toolchain technologies that can be used to develop a frontend for any programming language and a backend for any instruction set architecture. LLVM is designed around a language-independent intermediate representation (IR) that serves as a portable, high-level assembly language that can be optimized with a variety of transformations over multiple passes. The name LLVM originally stood for Low Level Virtual Machine, though the project has expanded and the name is no longer officially an initialism.

This repository contains the source code for LLVM, a toolkit for the
construction of highly optimized compilers, optimizers, and run-time
environments.

The LLVM project has multiple components. The core of the project is
itself called "LLVM". This contains all of the tools, libraries, and header
files needed to process intermediate representations and convert them into
object files. Tools include an assembler, disassembler, bitcode analyzer, and
bitcode optimizer.

# 0. Intro
This course will consist of 4 laboratory work. As part of the laboratory works, you will study all stages of compilation, starting with the generation of the AST tree, ending with code generation.

## Note
Recommended OS - Linux (WSL).

# 1. Clone repository
1. Create fork this repository
2. Clone local fork
```bash
git clone https://github.com/<your-github-name>/compiler-course-2025.git
cd compiler-course-2025/
git checkout -b <name-your-branch>
```

# 2. Setup environment
```bash
sudo apt-get update \
    && sudo apt-get install -q -y --no-install-recommends \
        git \
        cmake \
        ccache \
        mold \
        clang \
        build-essential \
        ninja-build \
        python3 \
        python3-pip \
        wget \
        vim
```

# 3. Build project
```bash
cmake -G Ninja -S llvm -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DLLVM_USE_LINKER=mold \
    -DLLVM_CCACHE_BUILD=ON \
    -DLLVM_ENABLE_ASSERTIONS=ON \
    -DLLVM_ENABLE_PROJECTS="clang;mlir" \
    -DLLVM_TARGETS_TO_BUILD=X86 # specify your target architecture (X86, Aarch64)
cmake --build build --config Release -j 4
```
# 4. Where to implement laboratory work?
Each directory has an implementation example.

## ClangAST lab
```bash
cd clang/compiler-course/ # for labs
```
```bash
cd clang/test/compiler-course/ # for tests
```
## LLVM IR lab
```bash
cd llvm/compiler-course/llvm-ir/ # for labs
```
```bash
cd llvm/test/compiler-course/ # for tests
```
## Backend lab
```bash
cd llvm/compiler-course/backend/ # for labs
```
```bash
cd llvm/test/compiler-course/ # for tests
```
## MLIR lab
```bash MLIR
cd mlir/compiler-course/ # for labs
```
```bash MLIR
cd mlir/test/compiler-course/ # for tests
```
# 5. Run tests
For all tests
```bash
cmake --build build --config Release -t \
    check-llvm-compiler-course \
    check-clang-compiler-course \
    check-mlir-compiler-course -j 4
```
For one test
```bash
./build/bin/llvm-lit -v /path/to/test_file
```
