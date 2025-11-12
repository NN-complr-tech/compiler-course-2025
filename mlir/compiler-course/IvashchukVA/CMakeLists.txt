set(Title "CallCounterPass")
set(Student "IvashchukVA")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

# ИСПРАВЛЕНИЕ: используем add_mlir_library вместо add_mlir_pass_plugin
add_mlir_library(${TARGET_NAME}
  ${SOURCES}
  
  DEPENDS
  MLIRFuncDialect
  MLIRPass
  
  LINK_LIBS PUBLIC
  MLIRFunc
  MLIRPass
  MLIRIR
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)