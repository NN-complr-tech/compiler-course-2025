set(Title "CallCounterPass")
set(Student "IvashchukVA")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

# Используем add_mlir_library с правильными зависимостями
add_mlir_library(${TARGET_NAME}
  ${SOURCES}
  
  LINK_LIBS PUBLIC
  MLIRFunc
  MLIRPass
  MLIRIR
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)