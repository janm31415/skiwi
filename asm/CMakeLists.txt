set(HDRS
asm_api.h
assembler.h
asmcode.h
namespace.h
)
	
set(SRCS
assembler.cpp
asmcode.cpp
)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

add_library(asm SHARED ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(asm
    PRIVATE
    )	
	
target_link_libraries(asm
    PRIVATE	
    )	