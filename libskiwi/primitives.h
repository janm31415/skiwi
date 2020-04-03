#pragma once

#include <asm/asmcode.h>
#include <asm/namespace.h>

#include <vector>

#include "compiler_options.h"

SKIWI_BEGIN

const std::vector<asmcode::operand>& get_argument_registers();

void compile_halt(asmcode& code, const compiler_options& options);
void compile_add1(asmcode& code, const compiler_options& options);
void compile_sub1(asmcode& code, const compiler_options& options);

void compile_file_exists(asmcode& code, const compiler_options& options);
void compile_getenv(asmcode& code, const compiler_options& options);
void compile_putenv(asmcode& code, const compiler_options& options);
void compile_eval(asmcode& code, const compiler_options& options);
void compile_load(asmcode& code, const compiler_options& options);
void compile_error(asmcode& code, const compiler_options& options);
void compile_is_eq(asmcode& code, const compiler_options& options);
void compile_is_eqv(asmcode& code, const compiler_options& options);
void compile_is_eqv_structurally(asmcode& code, const compiler_options& options);
void compile_is_equal(asmcode& code, const compiler_options& options);
void compile_add(asmcode& code, const compiler_options& options);
void compile_sub(asmcode& code, const compiler_options& options);
void compile_mul(asmcode& code, const compiler_options& options);
void compile_div(asmcode& code, const compiler_options& options);
void compile_max(asmcode& code, const compiler_options& options);
void compile_min(asmcode& code, const compiler_options& options);
void compile_equal(asmcode& code, const compiler_options& options);
void compile_not_equal(asmcode& code, const compiler_options& options);
void compile_less(asmcode& code, const compiler_options& options);
void compile_leq(asmcode& code, const compiler_options& options);
void compile_geq(asmcode& code, const compiler_options& options);
void compile_greater(asmcode& code, const compiler_options& options);
void compile_bitwise_and(asmcode& code, const compiler_options& options);
void compile_bitwise_not(asmcode& code, const compiler_options& options);
void compile_bitwise_or(asmcode& code, const compiler_options& options);
void compile_bitwise_xor(asmcode& code, const compiler_options& options);

void compile_make_promise(asmcode& code, const compiler_options& options);
void compile_compare_strings(asmcode& code, const compiler_options& options);
void compile_compare_strings_ci(asmcode& code, const compiler_options& options);
void compile_fx_equal(asmcode& code, const compiler_options& options);
void compile_fx_less(asmcode& code, const compiler_options& options);
void compile_fx_greater(asmcode& code, const compiler_options& options);
void compile_fx_leq(asmcode& code, const compiler_options& options);
void compile_fx_geq(asmcode& code, const compiler_options& options);
void compile_fx_add(asmcode& code, const compiler_options& options);
void compile_fx_sub(asmcode& code, const compiler_options& options);
void compile_fx_mul(asmcode& code, const compiler_options& options);
void compile_fx_div(asmcode& code, const compiler_options& options);
void compile_fx_add1(asmcode& code, const compiler_options& options);
void compile_fx_sub1(asmcode& code, const compiler_options& options);
void compile_fx_is_zero(asmcode& code, const compiler_options& options);
void compile_char_equal(asmcode& code, const compiler_options& options);
void compile_char_less(asmcode& code, const compiler_options& options);
void compile_char_greater(asmcode& code, const compiler_options& options);
void compile_char_leq(asmcode& code, const compiler_options& options);
void compile_char_geq(asmcode& code, const compiler_options& options);
void compile_char_to_fixnum(asmcode& code, const compiler_options& options);
void compile_fixnum_to_char(asmcode& code, const compiler_options& options);
void compile_closure(asmcode& code, const compiler_options& options);
void compile_closure_ref(asmcode& code, const compiler_options& options);
void compile_make_vector(asmcode& code, const compiler_options& options);
void compile_vector(asmcode& code, const compiler_options& options);
void compile_vector_length(asmcode& code, const compiler_options& options);
void compile_slot_ref(asmcode& code, const compiler_options& options);
void compile_slot_set(asmcode& code, const compiler_options& options);
void compile_vector_ref(asmcode& code, const compiler_options& options);
void compile_vector_set(asmcode& code, const compiler_options& options);
void compile_allocate_symbol(asmcode& code, const compiler_options& options);
void compile_make_string(asmcode& code, const compiler_options& options);
void compile_string(asmcode& code, const compiler_options& options);
void compile_string_length(asmcode& code, const compiler_options& options);
void compile_string_ref(asmcode& code, const compiler_options& options);
void compile_string_set(asmcode& code, const compiler_options& options);
void compile_list(asmcode& code, const compiler_options& options);
void compile_length(asmcode& code, const compiler_options& options);
void compile_set_car(asmcode& code, const compiler_options& options);
void compile_set_cdr(asmcode& code, const compiler_options& options);
void compile_make_port(asmcode& code, const compiler_options& options);
void compile_is_port(asmcode& code, const compiler_options& options);
void compile_is_input_port(asmcode& code, const compiler_options& options);
void compile_is_output_port(asmcode& code, const compiler_options& options);
void compile_read_char(asmcode& code, const compiler_options& options);
void compile_peek_char(asmcode& code, const compiler_options& options);
void compile_write_char(asmcode& code, const compiler_options& options);
void compile_write_string(asmcode& code, const compiler_options& options);
void compile_flush_output_port(asmcode& code, const compiler_options& options);
void compile_open_file(asmcode& code, const compiler_options& options);
void compile_close_file(asmcode& code, const compiler_options& options);
void compile_is_eof_object(asmcode& code, const compiler_options& options);
void compile_num2str(asmcode& code, const compiler_options& options);
void compile_str2num(asmcode& code, const compiler_options& options);
void compile_string_fill(asmcode& code, const compiler_options& options);
void compile_vector_fill(asmcode& code, const compiler_options& options);
void compile_substring(asmcode& code, const compiler_options& options);
void compile_string_append1(asmcode& code, const compiler_options& options);
void compile_string_hash(asmcode& code, const compiler_options& options);

void compile_ieee754_sign(asmcode& code, const compiler_options& options);
void compile_ieee754_exponent(asmcode& code, const compiler_options& options);
void compile_ieee754_mantissa(asmcode& code, const compiler_options& options);
void compile_ieee754_sin(asmcode& code, const compiler_options& options);
void compile_ieee754_cos(asmcode& code, const compiler_options& options);
void compile_ieee754_tan(asmcode& code, const compiler_options& options);
void compile_ieee754_asin(asmcode& code, const compiler_options& options);
void compile_ieee754_acos(asmcode& code, const compiler_options& options);
void compile_ieee754_atan1(asmcode& code, const compiler_options& options);
void compile_ieee754_log(asmcode& code, const compiler_options& options);
void compile_ieee754_round(asmcode& code, const compiler_options& options);
void compile_ieee754_truncate(asmcode& code, const compiler_options& options);
void compile_ieee754_sqrt(asmcode& code, const compiler_options& options);
void compile_ieee754_pi(asmcode& code, const compiler_options& options);
void compile_fixnum_expt(asmcode& code, const compiler_options& options);
void compile_flonum_expt(asmcode& code, const compiler_options& options);

void compile_not(asmcode& code, const compiler_options& options);
void compile_is_fixnum(asmcode& code, const compiler_options& options);
void compile_is_flonum(asmcode& code, const compiler_options& options);
void compile_is_nil(asmcode& code, const compiler_options& options);
void compile_is_vector(asmcode& code, const compiler_options& options);
void compile_is_pair(asmcode& code, const compiler_options& options);
void compile_is_string(asmcode& code, const compiler_options& options);
void compile_is_closure(asmcode& code, const compiler_options& options);
void compile_is_procedure(asmcode& code, const compiler_options&);
void compile_is_symbol(asmcode& code, const compiler_options& options);
void compile_is_promise(asmcode& code, const compiler_options& options);
void compile_is_zero(asmcode& code, const compiler_options& options);
void compile_is_char(asmcode& code, const compiler_options& options);
void compile_is_boolean(asmcode& code, const compiler_options& options);
void compile_member(asmcode& code, const compiler_options& options);
void compile_memv(asmcode& code, const compiler_options& options);
void compile_memq(asmcode& code, const compiler_options& ops);
void compile_assoc(asmcode& code, const compiler_options& options);
void compile_assv(asmcode& code, const compiler_options& options);
void compile_assq(asmcode& code, const compiler_options& ops);
void compile_apply(asmcode& code, const compiler_options& options);
void compile_cons(asmcode& code, const compiler_options& options);
void compile_car(asmcode& code, const compiler_options& options);
void compile_cdr(asmcode& code, const compiler_options& options);
void compile_reclaim_garbage(asmcode& code, const compiler_options& options);
void compile_reclaim(asmcode& code, const compiler_options& options);
void compile_arithmetic_shift(asmcode& code, const compiler_options& options);
void compile_quotient(asmcode& code, const compiler_options& options);
void compile_remainder(asmcode& code, const compiler_options& options);
void compile_fixnum_to_flonum(asmcode& code, const compiler_options& options);
void compile_flonum_to_fixnum(asmcode& code, const compiler_options& options);
void compile_string_copy(asmcode& code, const compiler_options& options);
void compile_symbol_to_string(asmcode& code, const compiler_options& options);
void compile_undefined(asmcode& code, const compiler_options& options);
void compile_quiet_undefined(asmcode& code, const compiler_options& options);

void compile_member_cmp_eq(asmcode& code, const compiler_options&);
void compile_member_cmp_eqv(asmcode& code, const compiler_options&);
void compile_member_cmp_equal(asmcode& code, const compiler_options&);
void compile_assoc_cmp_eq(asmcode& code, const compiler_options&);
void compile_assoc_cmp_eqv(asmcode& code, const compiler_options&);
void compile_assoc_cmp_equal(asmcode& code, const compiler_options&);
void compile_structurally_equal(asmcode& code, const compiler_options&, const std::string& label_name);
void compile_recursively_equal(asmcode& code, const compiler_options&, const std::string& label_name);
void compile_mark(asmcode& code, const compiler_options&);
void compile_not_equal_2(asmcode& code, const compiler_options& ops);
void compile_equal_2(asmcode& code, const compiler_options& options);
void compile_less_2(asmcode& code, const compiler_options& options);
void compile_leq_2(asmcode& code, const compiler_options& options);
void compile_geq_2(asmcode& code, const compiler_options& options);
void compile_greater_2(asmcode& code, const compiler_options& options);
void compile_bitwise_and_2(asmcode& code, const compiler_options& options);
void compile_bitwise_or_2(asmcode& code, const compiler_options& options);
void compile_bitwise_xor_2(asmcode& code, const compiler_options& options);
void compile_add_2(asmcode& code, const compiler_options& options);
void compile_subtract_2(asmcode& code, const compiler_options& options);
void compile_multiply_2(asmcode& code, const compiler_options& options);
void compile_divide_2(asmcode& code, const compiler_options& options);
void compile_max_2(asmcode& code, const compiler_options& options);
void compile_min_2(asmcode& code, const compiler_options& options);
void compile_fold_binary(asmcode& code, const compiler_options& options);
void compile_pairwise_compare(asmcode& code, const compiler_options& options);
void compile_apply_fake_cps_identity(asmcode& code, const compiler_options& options);

SKIWI_END
