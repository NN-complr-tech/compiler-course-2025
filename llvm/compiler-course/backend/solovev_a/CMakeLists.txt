set(Title "Lab_3_backend")
set(Student "Solovev_a")
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

target_include_directories(${TARGET_NAME} PUBLIC ${PATH_TO_X86})
set(LLVM_TEST_DEPENDS ${TARGET_NAME} ${LLVM_TEST_DEPENDS} PARENT_SCOPE)
