add_llvm_library(ClassInfoVisitorPlugin MODULE ClassInfoPlugin.cpp PLUGIN_TOOL clang)

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS
    Support
  )
  clang_target_link_libraries(ClassInfoVisitorPlugin PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "ClassInfoVisitorPlugin" ${CLANG_TEST_DEPS} PARENT_SCOPE)