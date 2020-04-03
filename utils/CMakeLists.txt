
set(HDRS
encoding.h
filename.h
namespace.h
utf8.h
utf8_checked.h
utf8_core.h
utf8_unchecked.h
)
	
set(SRCS
utils.cpp
)

set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

add_library(utils STATIC ${HDRS} ${SRCS})
source_group("Header Files" FILES ${HDRS})
source_group("Source Files" FILES ${SRCS})

target_include_directories(utils
    PRIVATE
    )	
	
target_link_libraries(utils
    PRIVATE	
    )	