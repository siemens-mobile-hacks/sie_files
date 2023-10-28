cmake_minimum_required(VERSION 3.14.1)

get_filename_component(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../include ABSOLUTE)
get_filename_component(LIB_DIR ${CMAKE_SOURCE_DIR}/../lib ABSOLUTE)

macro(push_library lib)
    set(LIBS "${LIBS} ${LIB_DIR}/${lib}")
endmacro()

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_C_COMPILER "arm-eabi-gcc")
set(CMAKE_LINKER "arm-eabi-ld")

include_directories(${INCLUDE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msoft-float -fshort-wchar -marm -mlittle-endian -mcpu=arm926ej-s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -nostdlib -pipe -fno-exceptions -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finput-charset=UTF-8 -fexec-charset=cp1251")

push_library(libcrt.a)
push_library(libcrt_helper.so)
push_library(libc-1.0.so)
push_library(libsie.so)
set(CMAKE_EXE_LINKER_FLAGS "-marmelf -zmax-page-size=1 -nostdlib --defsym __ex=0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -s -gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rpath-link ${LIB_DIR} ${LIBS}")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET>")

project(sieelf_sie_files)
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(sie_files.elf ${SOURCES})
