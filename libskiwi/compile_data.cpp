#include "compile_data.h"
#include "context.h"

SKIWI_BEGIN

compile_data create_compile_data(uint64_t heap_size, uint64_t globals_stack, uint32_t local_stack, context* p_ctxt)
  {
  compile_data cd;
  std::vector<asmcode::operand> usable_registers;
  
  usable_registers.push_back(asmcode::RCX);
  usable_registers.push_back(asmcode::RDX);
  usable_registers.push_back(asmcode::RSI);
  usable_registers.push_back(asmcode::RDI);
  usable_registers.push_back(asmcode::R8);
  usable_registers.push_back(asmcode::R9);
  usable_registers.push_back(asmcode::R12);
  usable_registers.push_back(asmcode::R14);
  
  cd.ra = std::make_unique<reg_alloc>(usable_registers, local_stack);
  cd.local_stack_size = local_stack;
  cd.globals_stack = globals_stack;
  cd.heap_size = heap_size;
  cd.first_ra_map_time_point_to_elapse = (uint64_t)-1;

  cd.p_ctxt = p_ctxt;
  return cd;
  }

SKIWI_END
