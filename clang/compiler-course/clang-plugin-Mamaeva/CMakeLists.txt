cmake_minimum_required(VERSION 3.13)
project(myClangPlugin)

set(Title "myClangPlugin")
set(Student "mamaeva_olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_ClangAST")

# Поиск LLVM и Clang
file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)
add_llvm_library(${TARGET_NAME} MODULE ${SOURCES} PLUGIN_TOOL clang)

# Поддержка разных платформ
if(UNIX AND NOT APPLE)
    # Настройки для Linux
elseif(APPLE)
    # Настройки для macOS
elseif(WIN32 OR CYGWIN)
    set(LLVM_LINK_COMPONENTS Support)
    clang_target_link_libraries(${TARGET_NAME} PRIVATE
        clangAST
        clangBasic
        clangFrontend
    )
endif()

set(CLANG_TEST_DEPS "${TARGET_NAME}" ${CLANG_TEST_DEPS} PARENT_SCOPE)
