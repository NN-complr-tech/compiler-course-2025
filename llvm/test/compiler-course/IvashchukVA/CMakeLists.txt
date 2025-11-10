add_lit_testsuite(check-llvm-compiler-course-IvashchukVA "Running the LLVM compiler course tests for IvashchukVA"
  ${CMAKE_CURRENT_BINARY_DIR}
  PARAMS ${LLVM_LIT_PARAMS}
  )

add_dependencies(check-llvm-compiler-course-IvashchukVA IvashchukVA_lab2)