cmake_minimum_required(VERSION 3.18)

project(snake VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor/hurdygurdy)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

if(MSVC)
    add_compile_options(/permissive- /GR- /D_CRT_SECURE_NO_WARNINGS /W4 /WX)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -Wall -Wextra -Wconversion -Wsign-conversion -pedantic -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wno-cpp")

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CLANG_VENDOR_FLAGS
            -Wno-nullability-extension
            -Wno-nullability-completeness
        )
    endif()

    find_program(MOLD_PROGRAM mold)
    if(MOLD_PROGRAM)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
    endif()
endif()

add_executable(snake
    src/snake.cpp
)
target_link_libraries(snake PRIVATE hurdygurdy)

install(TARGETS snake
    RUNTIME DESTINATION bin
)

