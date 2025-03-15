cmake_minimum_required(VERSION 3.10)
project(myClangPlugin)

# Указываем путь к LLVM
set(LLVM_DIR "/usr/lib/llvm-19/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Устанавливаем переменные для названия проекта, студента и группы
set(Title "myClangPlugin")
set(Student "mamaeva_olga")
set(Group "FIIT3")
set(TARGET_NAME "${Title}_${Student}_${Group}_ClangAST")

# Добавляем инклюды и флаги
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Ищем все исходные файлы в проекте
file(GLOB_RECURSE SOURCES *.cpp *.h *.hpp)

# Создаем LLVM-библиотеку (плагин)
add_library(${TARGET_NAME} MODULE ${SOURCES})

# Линкуем необходимые библиотеки Clang
target_link_libraries(${TARGET_NAME} PRIVATE
    clangAST
    clangBasic
    clangFrontend
)
