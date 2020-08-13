#pragma once

#include "namespace.h"

SKIWI_BEGIN

#define CONTEXT ASM::asmcode::R10
#define RBX_STORE ASM::asmcode::MEM_R10, 0
#define RDI_STORE ASM::asmcode::MEM_R10, 8
#define RSI_STORE ASM::asmcode::MEM_R10, 16
#define RSP_STORE ASM::asmcode::MEM_R10, 24
#define RBP_STORE ASM::asmcode::MEM_R10, 32
#define R12_STORE ASM::asmcode::MEM_R10, 40
#define R13_STORE ASM::asmcode::MEM_R10, 48
#define R14_STORE ASM::asmcode::MEM_R10, 56
#define R15_STORE ASM::asmcode::MEM_R10, 64

#define ALLOC ASM::asmcode::RBP
#define MEM_ALLOC ASM::asmcode::MEM_RBP
#define BYTE_MEM_ALLOC ASM::asmcode::BYTE_MEM_RBP

#define ALLOC_SAVED ASM::asmcode::MEM_R10, 72
#define LIMIT ASM::asmcode::MEM_R10, 80
#define FROM_SPACE ASM::asmcode::MEM_R10, 88
#define FROM_SPACE_END ASM::asmcode::MEM_R10, 96
#define TO_SPACE ASM::asmcode::MEM_R10, 104
#define TO_SPACE_END ASM::asmcode::MEM_R10, 112
#define FROMSPACE_RESERVE ASM::asmcode::MEM_R10, 128
#define TEMP_FLONUM ASM::asmcode::MEM_R10, 152
#define GC_SAVE ASM::asmcode::MEM_R10, 160
#define RSP_SAVE ASM::asmcode::MEM_R10, 184

#define GLOBALS ASM::asmcode::MEM_R10, 168
#define GLOBALS_END ASM::asmcode::MEM_R10, 176
#define LOCAL ASM::asmcode::MEM_R10, 136

#define BUFFER ASM::asmcode::MEM_R10, 192
#define DCVT ASM::asmcode::MEM_R10, 200
#define OCVT ASM::asmcode::MEM_R10, 208
#define XCVT ASM::asmcode::MEM_R10, 216
#define GCVT ASM::asmcode::MEM_R10, 224

#define STACK_TOP ASM::asmcode::MEM_R10, 232
#define STACK ASM::asmcode::MEM_R10, 240
#define STACK_SAVE ASM::asmcode::MEM_R10, 248

#define ERROR ASM::asmcode::MEM_R10, 256

#define LAST_GLOBAL_VARIABLE_USED ASM::asmcode::MEM_R10, 272


#define STACK_REGISTER ASM::asmcode::R13
#define STACK_REGISTER_MEM ASM::asmcode::MEM_R13

#define CONTINUE ASM::asmcode::RBX

#define CELLS(n) (n)*8

SKIWI_END
