cmake_minimum_required(VERSION 3.0.0)
project(peach)

add_compile_options(-Wall -Wextra -pedantic)

set(EXE
    peach    
)

set(SOURCES
    src/main.cpp
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PUBLIC include)
target_include_directories(${EXE} PUBLIC include/Evaluation)
target_include_directories(${EXE} PUBLIC include/Interpretation)
target_include_directories(${EXE} PUBLIC include/Tokenization)
target_include_directories(${EXE} PUBLIC include/Tokenization/Finders)
target_include_directories(${EXE} PUBLIC include/Tools)
set_property(TARGET ${EXE}
             PROPERTY CXX_STANDARD 17)
