set(Title "AVXLogicOptPass")
set(Student "Baranov_Aleksey")
set(Group "FIIT1")
set(TARGET_NAME "${Title}_${Student}_${Group}_BACKEND")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_llvm_pass_plugin(${TARGET_NAME}
  ${SOURCES}
  DEPENDS
    intrinsics_gen
    X86
  BUILDTREE_ONLY
)
set_target_properties(${TARGET_NAME} PROPERTIES
  PREFIX ""
  OUTPUT_NAME "AVXLogicOptPass_Baranov_Aleksey_FIIT1_BACKEND"
)

target_include_directories(${TARGET_NAME} PUBLIC ${PATH_TO_X86})
set(LLVM_TEST_DEPENDS ${TARGET_NAME} ${LLVM_TEST_DEPENDS} PARENT_SCOPE)