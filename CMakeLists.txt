cmake_minimum_required(VERSION 4.0)
project(SIEWLangC C)

set(CMAKE_C_STANDARD 11)

add_library(siew STATIC
        src/core/chunk.c
        src/core/memory.c
        src/core/value.c
        src/vm/debug.c
        src/vm/vm.c
        src/compiler/compiler.c
        src/compiler/scanner.c
        src/core/object.c
)

target_include_directories(siew PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_executable(SIEWLangC apps/siewc/main.c)
target_link_libraries(SIEWLangC PRIVATE siew)