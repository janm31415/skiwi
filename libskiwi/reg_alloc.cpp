#include "reg_alloc.h"

#include <cassert>

SKIWI_BEGIN

reg_alloc::reg_alloc(const std::vector<asmcode::operand>& usable_registers, uint32_t number_of_locals) : available_registers(usable_registers),
  nr_locals(number_of_locals)
  {  
  make_all_available();
  }

reg_alloc::~reg_alloc()
  {
  }

bool reg_alloc::free_register_available() const
  {
  return !free_registers.empty();
  }

bool reg_alloc::free_local_available() const
  {
  return !free_locals.empty();
  }

asmcode::operand reg_alloc::get_next_available_register()
  {
  if (!free_register_available())
    throw std::runtime_error("no register available");
  assert(!free_registers.empty());
  auto it = (--free_registers.end());  
  uint8_t reg_value = *it;
  free_registers.erase(it);
  return available_registers[reg_value];
  }

void reg_alloc::make_register_available(asmcode::operand reg)
  {
  assert(free_registers.find(register_to_index[reg]) == free_registers.end());
  assert(register_to_index.find(reg) != register_to_index.end());
  free_registers.insert(register_to_index.at(reg));
  }

uint32_t reg_alloc::get_next_available_local()
  {
  if (!free_local_available())
    throw std::runtime_error("no local available");
  assert(!free_locals.empty());
  auto it = (--free_locals.end());
  uint32_t local_value = *it;
  free_locals.erase(it);
  return local_value;
  }

void reg_alloc::make_local_available(uint32_t val)
  {
  assert(free_locals.find(val) == free_locals.end());  
  free_locals.insert(val);
  }

uint8_t reg_alloc::number_of_registers()
  {
  return (uint8_t)available_registers.size();
  }

uint32_t reg_alloc::number_of_locals()
  {
  return nr_locals;
  }

uint8_t reg_alloc::number_of_available_registers()
  {
  return (uint8_t)free_registers.size();
  }

uint32_t reg_alloc::number_of_available_locals()
  {
  return (uint32_t)free_locals.size();
  }

bool reg_alloc::is_free_register(asmcode::operand reg)
  {
  auto it = register_to_index.find(reg);
  if (it == register_to_index.end())
    return true;
  return (free_registers.find(it->second) != free_registers.end());
  }

bool reg_alloc::is_free_local(uint32_t local_id)
  {
  return (free_locals.find(local_id) != free_locals.end());
  }

void reg_alloc::make_all_available()
  {
  for (uint8_t i = 0; i < (uint8_t)available_registers.size(); ++i)
    {
    register_to_index[available_registers[i]] = i;
    free_registers.insert(i);
    }

  for (uint32_t i = 0; i < nr_locals; ++i)
    free_locals.insert(i);
  }


void reg_alloc::make_register_unavailable(asmcode::operand reg)
  {
  free_registers.erase(register_to_index[reg]);
  }

void reg_alloc::make_local_unavailable(uint32_t val)
  {
  free_locals.erase(val);
  }

SKIWI_END
