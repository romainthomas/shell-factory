cmake_minimum_required(VERSION 3.0)
include(ExternalProject)

# Shell-Factory
set(SHELL_FACTORY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SHELL_FACTORY_INCLUDE_DIR "${SHELL_FACTORY_SOURCE_DIR}/include")

# Shell Factory options
option(VERBOSE       "Set to 'ON' for verbose compilation commands" OFF)
option(STACK_REALIGN "Set to 'ON' to ensure stack alignment to a 16 bytes boundary (Intel only)." OFF)
option(NO_BUILTIN    "Does not use the compiler builtins for common memory operations." OFF)
option(OUTPUT_LIB    "Compiles to a shared library instead of a standard executable." OFF)
set(SHELL_COMMAND "/bin/sh" CACHE STRING "SET_ARGV0")

message(STATUS "System:    ${CMAKE_SYSTEM_NAME}")
message(STATUS "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

# Set default build type to RELEASE
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

set(CFLAGS)
set(GCC_FLAGS)
set(CLANG_FLAGS)
set(LD_FLAGS)


# Setup C Flags

list(APPEND CFLAGS
  -std=c++1y
  -ffreestanding
  -fshort-wchar
  -fshort-enums
  -fno-common
  -fno-rtti
  -fno-exceptions
  -fno-non-call-exceptions
  -fno-asynchronous-unwind-tables
  -fomit-frame-pointer
  -ffunction-sections
  -fdata-sections
  -fno-stack-protector
  -nostdlib
  -nodefaultlibs
  -fPIC
  -static
  -DSET_ARGV0=1
  -DCOMMAND='${SHELL_COMMAND}'
)


list(APPEND CFLAGS
  -Wall
  -Wno-unused-function
  -Wextra
  -Wfatal-errors
)

set(CFLAGS_DEBUG "-fno-lto -g")


if (STACK_REALIGN)
  message(STATUS "Stack alignment enabled")
  list(APPEND CFLAGS -mstackrealign)
endif()

if (VERBOSE)
  list(APPEND CFLAGS -v)
endif()

if (NO_BUILTIN)
  message(STATUS "Builtin disabled")
  list(APPEND CFLAGS -fno-builtin)
  list(APPEND CFLAGS -DNO_BUILTIN=1)
endif()

if (OUTPUT_LIB)
  list(APPEND CFLAGS -shared)
endif()

# GCC Specific flags
list(APPEND GCC_FLAGS
  -fno-toplevel-reorder
  -finline-functions
  -fno-jump-tables
  -fno-leading-underscore
  -flto
  -nodefaultlibs
  -Os
)

# Clang specific flags
list(APPEND CLANG_FLAGS
  -Oz
  -Wno-invalid-noreturn
)

# Linker flags
list(APPEND LD_FLAGS
  -Wl,-T${SHELL_FACTORY_SOURCE_DIR}/factory-elf.lds
  -Wl,--gc-sections
  -Wl,-N
  -Wl,--build-id=none
)

# Fix for Android
if (ANDROID_NDK_TOOLCHAIN_INCLUDED)
  list(APPEND LD_FLAGS
    -Wl,-L${ANDROID_SYSTEM_LIBRARY_PATH}/usr/lib
  )
  include_directories("${ANDROID_SYSTEM_LIBRARY_PATH}/usr/include")
endif()

string(REPLACE ";" " " GCC_FLAGS   "${GCC_FLAGS}")
string(REPLACE ";" " " CLANG_FLAGS "${CLANG_FLAGS}")
string(REPLACE ";" " " CFLAGS      "${CFLAGS}")
string(REPLACE ";" " " LD_FLAGS    "${LD_FLAGS}")

set(CMAKE_CXX_FLAGS "${CFLAGS}")
set(CMAKE_C_FLAGS   "${CFLAGS}")

message(STATUS "C Compiler ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "CXX Compiler ID: ${CMAKE_CXX_COMPILER_ID}")

if (CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_FLAGS}")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${GCC_FLAGS}")
endif()

if ("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLANG_FLAGS}")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${CLANG_FLAGS}")
endif()

message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG   "${CFLAGS_DEBUG} ${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CFLAGS_DEBUG} ${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS ${LD_FLAGS})

message(STATUS "Linker Flags [Shared]:     ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "Linker Flags [Static]:     ${CMAKE_STATIC_LINKER_FLAGS}")
message(STATUS "Linker Flags [Executable]: ${CMAKE_EXE_LINKER_FLAGS}")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_STATIC_LINKER_FLAGS "")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Install path prefix prepended on to install directories." FORCE)
endif()

# Generate tests
macro(ADD_SHELLCODE arg0 arg1)

  set(output_name "${arg0}_${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}.bin")
  string(TOLOWER ${output_name} output_name)
  add_executable(${output_name} ${arg1})

  target_include_directories(${output_name} PUBLIC "${SHELL_FACTORY_INCLUDE_DIR}")
  target_include_directories(${output_name} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

  install(TARGETS ${output_name} DESTINATION bin)
endmacro()


# Shellcodes
ADD_SHELLCODE("hello"     "${CMAKE_CURRENT_SOURCE_DIR}/hello.cc")
ADD_SHELLCODE("exit"      "${CMAKE_CURRENT_SOURCE_DIR}/exit.cc")
ADD_SHELLCODE("shellexec" "${CMAKE_CURRENT_SOURCE_DIR}/shellexec.cc")
ADD_SHELLCODE("memexec"   "${CMAKE_CURRENT_SOURCE_DIR}/memexec.cc")
ADD_SHELLCODE("dropexec"  "${CMAKE_CURRENT_SOURCE_DIR}/dropexec.cc")


