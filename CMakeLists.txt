cmake_minimum_required(VERSION 3.28.2)

include(../sdk/config.cmake)
project(sieelf_sie_files C)

if (APPLE)
    set(CMAKE_C_LINK_FLAGS "")
endif()

include_directories("../libsie/src/include")
link_libraries(crt crt_helper c gcc ft sie)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(sie_files ${SOURCES})
target_sdk_setup(sie_files ELKA)
target_link_directories(sie_files PUBLIC "../libsie/build/lib/ELKA")
target_compile_options(sie_files BEFORE PUBLIC -std=gnu99 -Wall -Werror)
