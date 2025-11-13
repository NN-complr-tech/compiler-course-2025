set(Title "CallCounterPass")
set(Student "IvashchukVA") 
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_mlir_library(${TARGET_NAME}
  ${SOURCES}
  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/IR
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Pass
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/Func/IR

  DEPENDS
  intrinsics_gen
  MLIRBuiltinLocationAttributesIncGen

  LINK_LIBS PUBLIC
  MLIRFuncDialect
  MLIRIR
  MLIRPass

  PLUGIN_TOOL
  mlir-opt
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)