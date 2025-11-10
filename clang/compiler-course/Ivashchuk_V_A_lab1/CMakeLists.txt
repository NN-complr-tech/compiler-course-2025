add_llvm_library(IvashchukVA_lab1 MODULE
  
IvashchukVA_lab1.cpp
  
  PLUGIN_TOOL
  clang
  )
  set(LLVM_LIT_COMPONENTS compiler-course)

add_lit_testsuite(check-clang-compiler-course-IvashchukVA "Running the clang compiler course tests for IvashchukVA"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${LLVM_LIT_PARAMS}
  )

add_dependencies(check-clang-compiler-course-IvashchukVA IvashchukVA_lab1)