cmake_minimum_required(VERSION 3.13)
project(myClangPlugin)

set(Title "myClangPlugin")
set(Student "mamaeva_olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_ClangAST")

# Поиск LLVM и Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS} ${CLANG_LIBRARY_DIRS})

# Указание стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Экспорт compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Поиск исходных файлов
file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

# Создание библиотеки
add_llvm_library(${TARGET_NAME} MODULE ${SOURCES} PLUGIN_TOOL clang)

# Поддержка разных платформ
if(UNIX AND NOT APPLE)
    # Настройки для Linux
    set(LLVM_LINK_COMPONENTS Support)
    clang_target_link_libraries(${TARGET_NAME} PRIVATE
        clangAST
        clangBasic
        clangFrontend
    )
elseif(APPLE)
    # Настройки для macOS
    set(LLVM_LINK_COMPONENTS Support)
    clang_target_link_libraries(${TARGET_NAME} PRIVATE
        clangAST
        clangBasic
        clangFrontend
    )
elseif(WIN32 OR CYGWIN)
    # Настройки для Windows и Cygwin
    set(LLVM_LINK_COMPONENTS Support)
    clang_target_link_libraries(${TARGET_NAME} PRIVATE
        clangAST
        clangBasic
        clangFrontend
    )
endif()

# Установка плагина
install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Логирование и отладка
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

set(CLANG_TEST_DEPS "${TARGET_NAME}" ${CLANG_TEST_DEPS} PARENT_SCOPE)
