set(Title "rem_pass")
set(Student "Mamaeva_Olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

set(SOURCES 
    rem_pass.cpp  
)

add_llvm_pass_plugin(${TARGET_NAME}
  ${SOURCES}
  DEPENDS
  MLIRArithDialect
  MLIRFuncDialect
  BUILDTREE_ONLY
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)
