cmake_minimum_required(VERSION 3.28.3)
project(inject)

# Set this to a valid LLVM installation dir
set(LT_LLVM_INSTALL_DIR "/usr/local" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths 
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM 18 REQUIRED CONFIG)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(Backdoor SHARED Backdoor.cpp)

