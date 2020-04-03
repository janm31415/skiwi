cmake_minimum_required(VERSION 2.8.11)
project (Skiwi)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY     "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(asm)
add_subdirectory(asm.tests)
add_subdirectory(libskiwi)

if (WIN32)
add_subdirectory(s)

add_subdirectory(libskiwi.tests)
add_subdirectory(utils)
endif (WIN32)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
