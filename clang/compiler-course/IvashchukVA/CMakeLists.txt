set(Title "lab1")
set(Student "IvashchukVA")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_ClangAST")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_llvm_library(${TARGET_NAME} MODULE ${SOURCES} PLUGIN_TOOL clang)

# Добавляем зависимости для линковки
target_link_libraries(${TARGET_NAME} PRIVATE
  clangAST
  clangBasic
  clangFrontend
)

set(CLANG_TEST_DEPS "${TARGET_NAME}" ${CLANG_TEST_DEPS} PARENT_SCOPE)