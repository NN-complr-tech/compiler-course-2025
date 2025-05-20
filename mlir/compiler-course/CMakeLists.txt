set(Title "RemPass")
set(Student "Mamaeva_Olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_mlir_pass_plugin(RemPass_Mamaeva_Olga_FIIT3_MLIR
  rem_pass.cpp  
  LINK_LIBS PUBLIC
  MLIRArithOps
  MLIRPass
  MLIRTransformUtils
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)
