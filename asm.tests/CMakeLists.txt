
set(HDRS
AssemblerTests.h
AsmCodeTests.h
test_assert.h
    )
	
set(SRCS
AssemblerTests.cpp
AsmCodeTests.cpp
test_assert.cpp
test.cpp
)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

add_definitions(-DMEMORY_LEAK_TRACKING)

add_executable(asm.tests ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(asm.tests
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/../
    )
	
target_link_libraries(asm.tests
    PRIVATE
    asm
    )	