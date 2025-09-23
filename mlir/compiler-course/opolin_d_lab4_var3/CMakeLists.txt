set(Title "RemLowPass")
set(Student "Opolin_Dmitry")
set(Group "FIIT2")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

add_llvm_pass_plugin(${TARGET_NAME}
    ${SOURCES}
    DEPENDS
    intrinsics_gen
    MLIRBuiltinLocationAttributesIncGen
    BUILDTREE_ONLY
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)
