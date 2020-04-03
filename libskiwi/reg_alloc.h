#pragma once

#include "libskiwi_api.h"

#include <asm/namespace.h>
#include <asm/asmcode.h>

#include <map>
#include <set>
#include <stdint.h>
#include <vector>


SKIWI_BEGIN

class reg_alloc
  {
  public:
    SKIWI_SCHEME_API reg_alloc(const std::vector<asmcode::operand>& usable_registers, uint32_t number_of_locals);
    SKIWI_SCHEME_API ~reg_alloc();

    bool free_register_available() const;
    bool free_local_available() const;
    
    asmcode::operand get_next_available_register();
    void make_register_available(asmcode::operand reg);

    uint32_t get_next_available_local();
    void make_local_available(uint32_t val);

    uint8_t number_of_registers();
    uint32_t number_of_locals();

    uint8_t number_of_available_registers();
    uint32_t number_of_available_locals();

    bool is_free_register(asmcode::operand reg);
    bool is_free_local(uint32_t local_id);

    void make_all_available();

    uint32_t number_of_locals() const { return nr_locals; }

    void make_register_unavailable(asmcode::operand reg);
    void make_local_unavailable(uint32_t val);

    const std::vector<asmcode::operand>& registers() const { return available_registers; }

  private:
    std::vector<asmcode::operand> available_registers;
    std::map<asmcode::operand, uint8_t> register_to_index;
    uint32_t nr_locals; 
    std::set<uint32_t> free_locals;
    std::set<uint8_t> free_registers;
  };


SKIWI_END
