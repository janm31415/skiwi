
set(HDRS
alpha_conversion.h
asm_aux.h
assignable_var_conversion.h
c_prim_decl.h
closure_conversion.h
cinput_conversion.h
cinput_data.h
compiler_options.h
compile_data.h
compile_error.h
compiler.h
concurrency.h
constant_folding.h
constant_propagation.h
cps_conversion.h
context.h
context_defs.h
debug_find.h
define_conversion.h
dump.h
environment.h
filename_setter.h
file_utils.h
format.h
free_var_analysis.h
global_define_env.h
globals.h
include_handler.h
inlines.h
inline_primitives_conversion.h
lambda_to_let_conversion.h
libskiwi.h
libskiwi_api.h
linear_scan.h
linear_scan_index.h
liveness_range.h
load_lib.h
macro_data.h
macro_expander.h
namespace.h
parse.h
preprocess.h
primitives.h
primitives_lib.h
quasiquote_conversion.h
quote_collector.h
quote_conversion.h
reader.h
reg_alloc.h
reg_alloc_map.h
repl_data.h
runtime.h
simplify_to_core.h
single_begin_conversion.h
syscalls.h
tail_call_analysis.h
tail_calls_check.h
tokenize.h
types.h
utf8.h
visitor.h
)
	
set(SRCS
alpha_conversion.cpp
asm_aux.cpp
assignable_var_conversion.cpp
closure_conversion.cpp
cinput_conversion.cpp
compiler_options.cpp
compile_data.cpp
compile_error.cpp
compiler.cpp
constant_folding.cpp
constant_propagation.cpp
cps_conversion.cpp
context.cpp
define_conversion.cpp
dump.cpp
filename_setter.cpp
format.cpp
free_var_analysis.cpp
global_define_env.cpp
globals.cpp
include_handler.cpp
inlines.cpp
inline_primitives_conversion.cpp
lambda_to_let_conversion.cpp
libskiwi.cpp
linear_scan.cpp
linear_scan_index.cpp
load_lib.cpp
macro_data.cpp
macro_expander.cpp
parse.cpp
preprocess.cpp
primitives.cpp
primitives_lib.cpp
quasiquote_conversion.cpp
quote_collector.cpp
quote_conversion.cpp
reader.cpp
reg_alloc.cpp
reg_alloc_map.cpp
repl_data.cpp
runtime.cpp
simplify_to_core.cpp
single_begin_conversion.cpp
syscalls.cpp
tail_call_analysis.cpp
tail_calls_check.cpp
tokenize.cpp
)

if (UNIX)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread -std=c++11")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread -std=c++11")
endif (UNIX)

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_library(libskiwi SHARED ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(libskiwi
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/..  
    )	    

target_link_libraries(libskiwi
    PRIVATE	
    asm
    )	
   
if (WIN32)
add_custom_command(TARGET libskiwi POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scm" "${CMAKE_CURRENT_BINARY_DIR}/scm")
endif (WIN32)

if (UNIX)
add_custom_command(TARGET libskiwi POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scm" "$<TARGET_FILE_DIR:asm.tests>/scm")
endif (UNIX)
   
