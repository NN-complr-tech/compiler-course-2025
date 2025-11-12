set(Title "CallCounterPass")
set(Student "IvashchukVA")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

# ИСПРАВЛЕНИЕ: правильные зависимости для MLIR
add_mlir_library(${TARGET_NAME}
  ${SOURCES}
  
  DEPENDS
  mlir-headers
  
  LINK_LIBS PUBLIC
  MLIRFuncDialect
  MLIRPass
  MLIRIR
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)