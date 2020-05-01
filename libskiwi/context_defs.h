#pragma once

#include <asm/namespace.h>

SKIWI_BEGIN

#define CONTEXT asmcode::R10
#define RBX_STORE asmcode::MEM_R10, 0
#define RDI_STORE asmcode::MEM_R10, 8
#define RSI_STORE asmcode::MEM_R10, 16
#define RSP_STORE asmcode::MEM_R10, 24
#define RBP_STORE asmcode::MEM_R10, 32
#define R12_STORE asmcode::MEM_R10, 40
#define R13_STORE asmcode::MEM_R10, 48
#define R14_STORE asmcode::MEM_R10, 56
#define R15_STORE asmcode::MEM_R10, 64

#define ALLOC asmcode::RBP
#define MEM_ALLOC asmcode::MEM_RBP
#define BYTE_MEM_ALLOC asmcode::BYTE_MEM_RBP

#define ALLOC_SAVED asmcode::MEM_R10, 72
#define LIMIT asmcode::MEM_R10, 80
#define FROM_SPACE asmcode::MEM_R10, 88
#define FROM_SPACE_END asmcode::MEM_R10, 96
#define TO_SPACE asmcode::MEM_R10, 104
#define TO_SPACE_END asmcode::MEM_R10, 112
#define FROMSPACE_RESERVE asmcode::MEM_R10, 128
#define TEMP_FLONUM asmcode::MEM_R10, 152
#define GC_SAVE asmcode::MEM_R10, 160
#define RSP_SAVE asmcode::MEM_R10, 184

#define GLOBALS asmcode::MEM_R10, 168
#define GLOBALS_END asmcode::MEM_R10, 176
#define LOCAL asmcode::MEM_R10, 136

#define BUFFER asmcode::MEM_R10, 192
#define DCVT asmcode::MEM_R10, 200
#define OCVT asmcode::MEM_R10, 208
#define XCVT asmcode::MEM_R10, 216
#define GCVT asmcode::MEM_R10, 224

#define STACK_TOP asmcode::MEM_R10, 232
#define STACK asmcode::MEM_R10, 240
#define STACK_SAVE asmcode::MEM_R10, 248

#define ERROR asmcode::MEM_R10, 256

#define LAST_GLOBAL_VARIABLE_USED asmcode::MEM_R10, 264


#define STACK_REGISTER asmcode::R13
#define STACK_REGISTER_MEM asmcode::MEM_R13

#define CONTINUE asmcode::RBX

#define CELLS(n) (n)*8

SKIWI_END
