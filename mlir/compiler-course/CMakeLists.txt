set(Title "rem_pass")
set(Student "Mamaeva_Olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_MLIR")

set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/rem_pass.cpp
)

add_llvm_pass_plugin(${TARGET_NAME}
    ${SOURCES}
    DEPENDS
    intrinsics_gen
    MLIRBuiltinLocationAttributesIncGen
    BUILDTREE_ONLY
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(MLIR_TEST_DEPENDS ${TARGET_NAME} ${MLIR_TEST_DEPENDS} PARENT_SCOPE)
