cmake_minimum_required(VERSION 3.0.0)
project(peach)

add_compile_options(-Wall -Wextra -pedantic -Werror)

set(EXE
    peach    
)

set(SOURCES
    src/main.cpp
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PUBLIC include)
set_property(TARGET ${EXE}
             PROPERTY CXX_STANDARD 17)
