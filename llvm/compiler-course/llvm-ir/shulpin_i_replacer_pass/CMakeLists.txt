set(Title "ReplacerPass")
set(Student "ShulpinIlya")
set(Group "FIIT1")
set(TARGET_NAME "${Title}_${Student}_${Group}_LLVM_IR")

if (NOT WIN32 AND NOT CYGWIN)
    file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)
    add_llvm_pass_plugin(${TARGET_NAME} ${SOURCES}
        DEPENDS
        intrinsics_gen
        BUILDTREE_ONLY
    )
    set(LLVM_TEST_DEPENDS ${TARGET_NAME} ${LLVM_TEST_DEPENDS} PARENT_SCOPE)
endif()
