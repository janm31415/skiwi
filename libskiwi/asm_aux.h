#pragma once

#include "namespace.h"
#include <asm/asmcode.h>
#include <string>

#include "types.h"

SKIWI_BEGIN

ASM::asmcode::operand get_mem_operand(ASM::asmcode::operand reg);
ASM::asmcode::operand get_byte_mem_operand(ASM::asmcode::operand reg);

std::string label_to_string(uint64_t lab);
void store_registers(ASM::asmcode& code);
void load_registers(ASM::asmcode& code);

int64_t int2fixnum(int64_t i);
int64_t fixnum2int(int64_t i);

void break_point(ASM::asmcode& code);

void error_label(ASM::asmcode& code, const std::string& label_name, runtime_error re);

void jump_short_if_arg_is_not_block(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_flonum(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_closure(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_vector(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_symbol(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_string(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_pair(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_port(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_short_if_arg_does_not_point_to_promise(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);

void jump_if_arg_is_not_block(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_flonum(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_closure(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_vector(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_symbol(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_string(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_pair(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_port(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);
void jump_if_arg_does_not_point_to_promise(ASM::asmcode& code, ASM::asmcode::operand arg, ASM::asmcode::operand free_reg, const std::string& label_name);

/*assumes RAX contains the extra heap size requested*/
void check_heap(ASM::asmcode& code, runtime_error re);


void save_before_foreign_call(ASM::asmcode& code);
void restore_after_foreign_call(ASM::asmcode& code);

void align_stack(ASM::asmcode& code);
void restore_stack(ASM::asmcode& code);

void copy_string_to_buffer(ASM::asmcode& code);
void allocate_buffer_as_string(ASM::asmcode& code);

void raw_string_length(ASM::asmcode& code, ASM::asmcode::operand string_reg);
void string_length(ASM::asmcode& code, ASM::asmcode::operand string_reg);

void push(ASM::asmcode& code, ASM::asmcode::operand reg);
void pop(ASM::asmcode& code, ASM::asmcode::operand reg);

SKIWI_END
