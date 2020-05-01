#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include <asm/asmcode.h>
#include <stdint.h>

#define SKIWI_VARIABLE_DEBUG_STACK_SIZE 5

SKIWI_BEGIN

struct context {
  void* rbx; // offset 0
  void* rdi; // offset 8
  void* rsi; // offset 16
  void* rsp; // offset 24
  void* rbp; // offset 32
  void* r12; // offset 40
  void* r13; // offset 48
  void* r14; // offset 56
  void* r15; // offset 64  
  uint64_t* alloc; // offset 72
  uint64_t* limit; // offset 80
  uint64_t* from_space; // offset 88
  uint64_t* from_space_end; // offset 96
  uint64_t* to_space; // offset 104
  uint64_t* to_space_end; // offset 112
  uint64_t total_heap_size; // offset 120
  uint64_t from_space_reserve; // offset 128
  uint64_t* locals; // offset 136
  uint64_t number_of_locals; // offset 144
  uint64_t* temporary_flonum; // offset 152
  uint64_t* gc_save; //offset 160
  uint64_t* globals; // offset 168
  uint64_t* globals_end; //offset 176
  uint64_t* rsp_save; // offset 184
  uint64_t* buffer; // offset 192
  uint64_t* dcvt; // offset 200
  uint64_t* ocvt; // offset 208
  uint64_t* xcvt; // offset 216
  uint64_t* gcvt; // offset 224  
  uint64_t* stack_top; // offset 232
  uint64_t* stack; // offset 240
  uint64_t* stack_save; // offset 248
  uint64_t* error_label; // offset 256

  uint64_t last_global_variable_used[SKIWI_VARIABLE_DEBUG_STACK_SIZE];

  uint64_t* memory_allocated;
  };


SKIWI_SCHEME_API context create_context(uint64_t heap_size, uint64_t globals_stack, uint32_t local_stack, uint64_t scheme_stack);
SKIWI_SCHEME_API void destroy_context(const context& ctxt);

SKIWI_END
