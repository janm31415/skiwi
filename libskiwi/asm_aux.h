#pragma once

#include <asm/namespace.h>
#include <asm/asmcode.h>
#include <string>

#include "types.h"

SKIWI_BEGIN

asmcode::operand get_mem_operand(asmcode::operand reg);
asmcode::operand get_byte_mem_operand(asmcode::operand reg);

std::string label_to_string(uint64_t lab);
void store_registers(asmcode& code);
void load_registers(asmcode& code);

int64_t int2fixnum(int64_t i);
int64_t fixnum2int(int64_t i);

void break_point(asmcode& code);

void error_label(asmcode& code, const std::string& label_name, runtime_error re);

void jump_short_if_arg_is_not_block(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_flonum(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_closure(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_vector(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_symbol(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_string(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_pair(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_port(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_promise(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);

void jump_if_arg_is_not_block(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_flonum(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_closure(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_vector(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_symbol(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_string(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_pair(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_port(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_promise(asmcode& code, asmcode::operand arg, asmcode::operand free_reg, const std::string& label_name);

/*assumes RAX contains the extra heap size requested*/
void check_heap(asmcode& code, runtime_error re);


void save_before_foreign_call(asmcode& code);
void restore_after_foreign_call(asmcode& code);

void align_stack(asmcode& code);
void restore_stack(asmcode& code);

void copy_string_to_buffer(asmcode& code);
void allocate_buffer_as_string(asmcode& code);

void raw_string_length(asmcode& code, asmcode::operand string_reg);
void string_length(asmcode& code, asmcode::operand string_reg);

void push(asmcode& code, asmcode::operand reg);
void pop(asmcode& code, asmcode::operand reg);

SKIWI_END
