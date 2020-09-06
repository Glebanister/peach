cmake_minimum_required(VERSION 3.0.0)
project(peach)

add_compile_options(-Wall -Wextra -pedantic)

set(EXE
    peach
)

set(SOURCES
    cli/src/Main.cpp
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE core/include)
target_include_directories(${EXE} PRIVATE core/include/Evaluation)
target_include_directories(${EXE} PRIVATE core/include/Interpretation)
target_include_directories(${EXE} PRIVATE core/include/Tokenization)
target_include_directories(${EXE} PRIVATE core/include/Tokenization/Finders)
target_include_directories(${EXE} PRIVATE core/include/Tools)

target_include_directories(${EXE} PRIVATE cli/include)

set_property(TARGET ${EXE}
             PROPERTY CXX_STANDARD 17)
