#pragma once

/*

The machine registers are used in the following manner:

     Register name                          Usage
    --------------------------------------+--------------------------------------
     rax, r11, r15                          Temporary registers
     r10                                    Pointer to context
     rdx, rsi, rdi, r8, r9, r12             Argument registers
     rbx                                    Holds currently executing closure
     rcx                                    Holds current continuation
     rbp                                    Allocation-pointer
     r13                                    Allocation-limit
     r14                                    Contains the internal value for "#f"

*/

#include <stdint.h>
#include <cassert>

#define fixnum_mask  1
#define fixnum_tag   0
#define fixnum_shift 1

#define bool_mask   255 
#define bool_f       7      // 0 0 0 0 0 1 1 1
#define bool_t      15      // 0 0 0 0 1 1 1 1
#define char_mask   255
#define char_tag    23      // 0 0 0 1 0 1 1 1
#define nil         31      // 0 0 0 1 1 1 1 1
#define undefined_mask 255
#define undefined   39      // 0 0 1 0 0 1 1 1
#define error_mask 255
#define error_tag    47     // 0 0 1 0 1 1 1 1
#define unalloc_mask 255
#define unalloc_tag 63      // 0 0 1 1 1 1 1 1
//free 55                      0 0 1 1 0 1 1 1
#define eof_mask 255
#define eof_tag 71          // 0 1 0 0 0 1 1 1
#define quiet_undefined_mask 255
#define quiet_undefined 79  // 0 1 0 0 1 1 1 1
#define unresolved_mask 255
#define unresolved_tag 87   // 0 1 0 1 0 1 1 1
#define reserved_mask 255
#define reserved_tag 95     // 0 1 0 1 1 1 1 1


#define free_address_mask 7
#define free_address_tag 1 // 0 0 0 1


#define procedure_mask 7
#define procedure_tag  3  // 0 0 1 1

#define block_mask      7
#define block_tag       5 // 0 1 0 1
#define block_shift    56
#define block_header_mask      127

// first the "simple" types
#define flonum_tag       1
#define string_tag       2
#define symbol_tag       3
// in the middle the closure
#define closure_tag      4
// then the "complicated" types (i.e. they can refer to other types)
#define pair_tag         5
#define vector_tag       6
#define port_tag         7
#define promise_tag      8
#define block_size_mask 0xFFFFFFFFFFFFFF
#define block_mask_bit 0x8000000000000000

enum runtime_error
  {
  re_silent,
  re_lambda_invalid_number_of_arguments,
  re_add_contract_violation,
  re_add1_contract_violation,
  re_equal_contract_violation,
  re_not_equal_contract_violation,
  re_less_contract_violation,
  re_leq_contract_violation,
  re_greater_contract_violation,
  re_geq_contract_violation,
  re_div_contract_violation,
  re_mul_contract_violation,
  re_sub_contract_violation,
  re_sub1_contract_violation,
  re_invalid_program_termination,
  re_closure_expected,
  re_vector_ref_contract_violation,
  re_vector_ref_out_of_bounds,
  re_vector_set_contract_violation,
  re_vector_set_out_of_bounds,
  re_closure_ref_contract_violation,
  re_closure_ref_out_of_bounds,
  re_is_zero_contract_violation,
  re_cons_contract_violation,
  re_car_contract_violation,
  re_cdr_contract_violation,
  re_vector_contract_violation,
  re_make_vector_contract_violation,
  re_vector_length_contract_violation,
  re_string_contract_violation,
  re_make_string_contract_violation,
  re_string_length_contract_violation,
  re_string_ref_contract_violation,
  re_string_ref_out_of_bounds,
  re_string_set_contract_violation,
  re_string_set_out_of_bounds,
  re_eq_contract_violation,
  re_eqv_contract_violation,
  re_eqvstruct_contract_violation,
  re_isequal_contract_violation,
  re_length_contract_violation,
  re_set_car_contract_violation,
  re_set_cdr_contract_violation,
  re_fixnum_to_char_contract_violation,
  re_char_to_fixnum_contract_violation,
  re_char_equal_contract_violation,
  re_char_less_contract_violation,
  re_char_greater_contract_violation,
  re_char_leq_contract_violation,
  re_char_geq_contract_violation,
  re_fx_equal_contract_violation,
  re_fx_less_contract_violation,
  re_fx_greater_contract_violation,
  re_fx_leq_contract_violation,
  re_fx_geq_contract_violation,
  re_fx_add_contract_violation,
  re_fx_sub_contract_violation,
  re_fx_mul_contract_violation,
  re_fx_div_contract_violation,
  re_fx_add1_contract_violation,
  re_fx_sub1_contract_violation,
  re_fx_is_zero_contract_violation,
  re_bitwise_and_contract_violation,
  re_bitwise_not_contract_violation,
  re_bitwise_or_contract_violation,
  re_bitwise_xor_contract_violation,
  re_make_vector_heap_overflow,
  re_closure_heap_overflow,
  re_vector_heap_overflow,
  re_make_string_heap_overflow,
  re_string_heap_overflow,
  re_list_heap_overflow,
  re_cons_heap_overflow,
  re_symbol_heap_overflow,
  re_flonum_heap_overflow,
  re_foreign_call_contract_violation,
  re_memv_contract_violation,
  re_memq_contract_violation,
  re_member_contract_violation,
  re_assv_contract_violation,
  re_assq_contract_violation,
  re_assoc_contract_violation,
  re_apply_contract_violation,
  re_make_port_contract_violation,
  re_make_port_heap_overflow,
  re_write_char_contract_violation,
  re_flush_output_port_contract_violation,
  re_ieee754_sign_contract_violation,
  re_ieee754_exponent_contract_violation,
  re_ieee754_mantissa_contract_violation,
  re_max_contract_violation,
  re_min_contract_violation,
  re_arithmetic_shift_contract_violation,
  re_quotient_contract_violation,
  re_remainder_contract_violation,
  re_fixnum_to_flonum_contract_violation,
  re_flonum_to_fixnum_contract_violation,
  re_fixnum_to_flonum_heap_overflow,
  re_ieee754_sin_contract_violation,
  re_ieee754_cos_contract_violation,
  re_ieee754_tan_contract_violation,
  re_ieee754_asin_contract_violation,
  re_ieee754_acos_contract_violation,
  re_ieee754_atan1_contract_violation,
  re_ieee754_log_contract_violation,
  re_ieee754_round_contract_violation,
  re_ieee754_truncate_contract_violation,
  re_ieee754_sqrt_contract_violation,
  re_ieee754_pi_contract_violation,
  re_fixnum_expt_contract_violation,
  re_flonum_expt_contract_violation,
  re_is_port_contract_violation,
  re_is_input_port_contract_violation,
  re_is_output_port_contract_violation,
  re_open_file_contract_violation,
  re_close_file_contract_violation,
  re_port_ref_contract_violation,
  re_port_ref_out_of_bounds,
  re_read_char_contract_violation,
  re_peek_char_contract_violation,
  re_num2str_contract_violation,
  re_str2num_contract_violation,
  re_write_string_contract_violation,
  re_string_copy_contract_violation,
  re_symbol_to_string_contract_violation,
  re_symbol_to_string_heap_overflow,
  re_string_copy_heap_overflow,
  re_compare_strings_contract_violation,
  re_compare_strings_ci_contract_violation,
  re_vector_fill_contract_violation,
  re_string_fill_contract_violation,
  re_substring_contract_violation,
  re_substring_heap_overflow,
  re_substring_out_of_bounds,
  re_string_append_contract_violation,
  re_string_append_heap_overflow,
  re_string_hash_contract_violation,
  re_allocate_symbol_contract_violation,
  re_allocate_symbol_heap_overflow,
  re_make_promise_contract_violation,
  re_make_promise_heap_overflow,
  re_slot_ref_out_of_bounds,
  re_slot_ref_contract_violation,
  re_slot_set_out_of_bounds,
  re_slot_set_contract_violation,
  re_division_by_zero,
  re_heap_full,
  re_load_contract_violation,
  re_eval_contract_violation,
  re_getenv_contract_violation,
  re_getenv_heap_overflow,
  re_putenv_contract_violation,
  re_file_exists_contract_violation
  };

enum block_type
  {
  T_FLONUM,
  T_CLOSURE,
  T_PAIR,
  T_VECTOR,
  T_STRING,
  T_PORT,
  T_PROMISE
  };

inline uint64_t make_block_header(uint64_t size, block_type type)
  {
  switch (type)
    {
    case T_FLONUM: return size | ((uint64_t)flonum_tag << block_shift);
    case T_CLOSURE: return size | ((uint64_t)closure_tag << block_shift);
    case T_PAIR: return size | ((uint64_t)pair_tag << block_shift);
    case T_VECTOR: return size | ((uint64_t)vector_tag << block_shift);
    case T_STRING: return size | ((uint64_t)string_tag << block_shift);
    case T_PORT: return size | ((uint64_t)port_tag << block_shift);
    case T_PROMISE: return size | ((uint64_t)promise_tag << block_shift);
    default: return 0;
    }
  }

inline uint64_t* get_address_from_block(uint64_t rax)
  {
  assert((rax & block_mask) == block_tag);
  uint64_t address_location = rax & 0xFFFFFFFFFFFFFFF8;
  return reinterpret_cast<uint64_t*>(address_location);
  }

inline uint64_t get_block_size(uint64_t block_header)
  {
  return (block_header & block_size_mask);
  }

inline bool block_header_is_flonum(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == flonum_tag;
  }

inline bool block_header_is_closure(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == closure_tag;
  }

inline bool block_header_is_pair(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == pair_tag;
  }

inline bool block_header_is_vector(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == vector_tag;
  }

inline bool block_header_is_string(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == string_tag;
  }

inline bool block_header_is_symbol(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == symbol_tag;
  }

inline bool block_header_is_port(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == port_tag;
  }

inline bool block_header_is_promise(uint64_t block_header)
  {
  return ((block_header >> block_shift) & block_header_mask) == promise_tag;
  }