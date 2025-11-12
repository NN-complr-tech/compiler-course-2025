set(Title "lab1")
set(Student "IvashchukVA")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_ClangAST")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_clang_library(${TARGET_NAME}
  ${SOURCES}
  DEPENDS
  intrinsics_gen
  BUILDTREE_ONLY
)

set(CLANG_TEST_DEPENDS ${TARGET_NAME} ${CLANG_TEST_DEPENDS} PARENT_SCOPE)