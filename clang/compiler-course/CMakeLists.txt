cmake_minimum_required(VERSION 3.13)
project(CompilerCourseClangAST LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_llvm_executable(compiler-course-clangast
    ClangASTTool.cpp
)

target_link_libraries(compiler-course-clangast
    PRIVATE
    clangTooling
    clangBasic
    clangAST
    clangFrontend
)
