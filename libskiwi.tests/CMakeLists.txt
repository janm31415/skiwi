
set(HDRS
test_assert.h
compile_tests.h
compile_vm_tests.h
conversion_tests.h
format_tests.h
parse_tests.h
tokenize_tests.h
    )
	
set(SRCS
test_assert.cpp
test.cpp
compile_tests.cpp
compile_vm_tests.cpp
conversion_tests.cpp
format_tests.cpp
parse_tests.cpp
tokenize_tests.cpp
)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_definitions(-DMEMORY_LEAK_TRACKING)

add_executable(libskiwi.tests ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

if (WIN32)
add_custom_command(TARGET libskiwi.tests POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" ${CMAKE_CURRENT_BINARY_DIR}/data)
endif (WIN32)

if (UNIX)
add_custom_command(TARGET libskiwi.tests POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/data" "$<TARGET_FILE_DIR:libskiwi.tests>/data")
endif (UNIX)



target_include_directories(libskiwi.tests
    PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../
    )
	
target_link_libraries(libskiwi.tests
    PRIVATE
    asm
    libskiwi
    )	