cmake_minimum_required(VERSION 2.8.11)
project (skiwi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADING "std" CACHE STRING  "Choose your threading library: tbb is Intel's Threading Building Blocks library, ppl is Windows' concurrency library, std uses std::thread, and none does not use threading.")
 
set_property(CACHE THREADING PROPERTY STRINGS tbb ppl std none)

add_subdirectory(asm)
add_subdirectory(asm.tests)

if (NOT TARGET jtk)
add_subdirectory(jtk)
endif()

add_subdirectory(libskiwi)
add_subdirectory(life)
add_subdirectory(s)
add_subdirectory(libskiwi.tests)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
