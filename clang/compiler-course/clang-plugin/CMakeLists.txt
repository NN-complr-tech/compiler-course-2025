cmake_minimum_required(VERSION 3.10)
project(myClangPlugin)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(myClangPlugin MODULE myClangPlugin.cpp)
target_link_libraries(myClangPlugin PRIVATE clangAST clangBasic clangFrontend clangSerialization)
