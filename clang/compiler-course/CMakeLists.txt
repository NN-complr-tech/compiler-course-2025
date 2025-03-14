cmake_minimum_required(VERSION 3.14)
project(ImplicitCastPlugin)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_library(ImplicitCastPlugin MODULE
    ImplicitCastPlugin.cpp
)

target_link_libraries(ImplicitCastPlugin
    PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
)

install(TARGETS ImplicitCastPlugin DESTINATION lib)