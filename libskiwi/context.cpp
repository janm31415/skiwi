#include "context.h"
#include "types.h"
SKIWI_BEGIN

context create_context(uint64_t heap_size, uint64_t globals_stack, uint32_t local_stack, uint64_t scheme_stack)
  {
  context c;
  // The number of locals is not that large, but the buffer for the locals is large, as it equals the heapsize/4.
  // The reason is that we want to be able to do something like (apply + long-list).
  // The apply method will rewrite this as (+ a b c ... ) where a,b,c are the elements of long-list.
  // Theoretically this could be a pretty long list that overflows the local buffer if we only provide for local_stack memory places.
  // Therefore we provide a large local stack buffer.
  uint64_t local_stack_buffer = heap_size / 4;
  if (local_stack > local_stack_buffer)
    local_stack_buffer = (uint32_t)local_stack;
  uint64_t total_size = (uint64_t)5 + (uint64_t)256 + (uint64_t)3 + (uint64_t)8 + (uint64_t)local_stack_buffer + globals_stack + heap_size + scheme_stack;
  c.memory_allocated = new uint64_t[total_size];
  uint64_t* ptr = c.memory_allocated;
  for (uint64_t i = 0; i < total_size; ++i, ++ptr)
    *ptr = unalloc_tag;
  c.dcvt = c.memory_allocated;
  c.ocvt = c.dcvt + 1;
  c.xcvt = c.ocvt + 1;
  c.gcvt = c.xcvt + 1;
  c.buffer = c.gcvt + 1;
  c.rsp_save = c.buffer + 256;
  c.temporary_flonum = c.rsp_save + 1;
  c.gc_save = c.temporary_flonum + 2;
  c.locals = c.gc_save + 8;
  c.globals = c.locals + (uint64_t)local_stack_buffer;
  c.globals_end = c.globals + globals_stack;
  c.stack_top = c.globals + globals_stack;
  c.stack = c.stack_top;
  c.stack_end = c.stack + scheme_stack;
  c.from_space = c.globals + globals_stack + scheme_stack;
  c.to_space = c.from_space + (heap_size / 2);
  c.total_heap_size = heap_size;
  c.alloc = c.from_space;
  c.from_space_reserve = (c.total_heap_size / 2) * 1 / 8;
  c.from_space_end = c.from_space + (heap_size / 2);
  c.to_space_end = c.to_space + (heap_size / 2);
  c.limit = c.from_space_end - c.from_space_reserve;
  c.number_of_locals = (uint64_t)local_stack;

  c.temporary_flonum[0] = make_block_header(1, T_FLONUM);

  for (int i = 0; i < SKIWI_VARIABLE_DEBUG_STACK_SIZE; ++i)
    {
    c.last_global_variable_used[i] = (uint64_t)-1;
    }

  *c.dcvt = (uint64_t)'%' | (((uint64_t)'l') << 8) | (((uint64_t)'l') << 16) | (((uint64_t)'d') << 24);
  *c.ocvt = (uint64_t)'%' | (((uint64_t)'l') << 8) | (((uint64_t)'l') << 16) | (((uint64_t)'o') << 24);
  *c.xcvt = (uint64_t)'%' | (((uint64_t)'l') << 8) | (((uint64_t)'l') << 16) | (((uint64_t)'x') << 24);
  *c.gcvt = (uint64_t)'%' | (((uint64_t)'.') << 8) | (((uint64_t)'2') << 16) | (((uint64_t)'0') << 24) | (((uint64_t)'g') << 32);
  return c;
  }


void destroy_context(context& ctxt)
  {
  delete[] ctxt.memory_allocated;
  ctxt.memory_allocated = nullptr;
  }

context clone_context(const context& ctxt)
  {
  uint64_t heap_size = ctxt.total_heap_size;
  uint64_t globals_stack = ctxt.globals_end - ctxt.globals;
  uint64_t local_stack = ctxt.globals - ctxt.locals;
  uint64_t scheme_stack = ctxt.stack_end - ctxt.stack_top;
  context c = create_context(heap_size, globals_stack, (uint32_t)local_stack, scheme_stack);

  uint64_t total_size = (uint64_t)5 + (uint64_t)256 + (uint64_t)3 + (uint64_t)8 + (uint64_t)local_stack + globals_stack + heap_size + scheme_stack;

  for (uint64_t i = 0; i < total_size; ++i)
    c.memory_allocated[i] = ctxt.memory_allocated[i];

  return c;
  }

SKIWI_END
