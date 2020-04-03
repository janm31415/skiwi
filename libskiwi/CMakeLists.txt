

if (WIN32)
  set(TBB_INCLUDE_DIR "C:/Program Files/TBB/include")
  set(TBB_LIBRARIES "C:/Program Files/TBB/lib/intel64/vc14/tbb.lib" "C:/Program Files/TBB/lib/intel64/vc14/tbb_debug.lib")
endif (WIN32)


if (UNIX)
  set(TBB_INCLUDE_DIR "/usr/local/include" "/usr/include")
  set(TBB_LIBRARIES)
endif (UNIX)

set(HDRS
alpha_conversion.h
asm_aux.h
assignable_var_conversion.h
c_prim_decl.h
closure_conversion.h
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
exepath.h
filename_setter.h
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
visitor.h
)
	
set(SRCS
alpha_conversion.cpp
asm_aux.cpp
assignable_var_conversion.cpp
closure_conversion.cpp
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

set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")

# general build definitions
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_library(libskiwi SHARED ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(libskiwi
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils
    ${TBB_INCLUDE_DIR}
    )	
    
	
target_link_libraries(libskiwi
    PRIVATE	
    asm
    ${TBB_LIBRARIES}
    )	

add_custom_command(TARGET libskiwi POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scm" "${CMAKE_CURRENT_BINARY_DIR}/scm")

add_custom_command(TARGET libskiwi POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/scm" "$<TARGET_FILE_DIR:libskiwi>/scm")