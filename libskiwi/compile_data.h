#pragma once

#include <asm/namespace.h>
#include <stdint.h>
#include "environment.h"
#include "reg_alloc.h"
#include "reg_alloc_map.h"
#include "libskiwi_api.h"
#include <string>
#include <memory>

SKIWI_BEGIN

struct context;

struct compile_data
  {
  std::unique_ptr<reg_alloc> ra;
  reg_alloc_map ra_map;
  uint64_t first_ra_map_time_point_to_elapse; // helper variable for speeding up method register_allocation_expire_old_intervals
  uint32_t local_stack_size;
  uint64_t globals_stack;
  uint64_t heap_size;
  std::string halt_label;
  context* p_ctxt;
  };


SKIWI_SCHEME_API compile_data create_compile_data(uint64_t heap_size, uint64_t globals_stack, uint32_t local_stack, context* p_ctxt);

SKIWI_END
