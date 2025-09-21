cmake_minimum_required(VERSION 3.13)
project(CompilerCourseMLIR LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MLIR REQUIRED CONFIG)
message(STATUS "Using MLIRConfig at: ${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
include(TableGen)
include(AddMLIR)

include_directories(${MLIR_INCLUDE_DIRS})
add_definitions(${MLIR_DEFINITIONS})

add_mlir_library(CompilerCourseMLIR
  FoldAddZero.cpp
  LINK_LIBS PUBLIC
    MLIRIR
    MLIRArithDialect
    MLIRTransforms
)
