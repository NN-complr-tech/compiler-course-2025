add_lit_testsuite(check-clang-compiler-course-IvashchukVA "Running the clang compiler course tests for IvashchukVA"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${LLVM_LIT_PARAMS}
  )

add_dependencies(check-clang-compiler-course-IvashchukVA IvashchukVA_lab1)