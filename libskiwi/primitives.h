#pragma once

#include <asm/asmcode.h>
#include "namespace.h"

#include <vector>

#include "compiler_options.h"

SKIWI_BEGIN

const std::vector<ASM::asmcode::operand>& get_argument_registers();

void compile_halt(ASM::asmcode& code, const compiler_options& options);
void compile_add1(ASM::asmcode& code, const compiler_options& options);
void compile_sub1(ASM::asmcode& code, const compiler_options& options);

void compile_file_exists(ASM::asmcode& code, const compiler_options& options);
void compile_getenv(ASM::asmcode& code, const compiler_options& options);
void compile_current_seconds(ASM::asmcode& code, const compiler_options& options);
void compile_current_milliseconds(ASM::asmcode& code, const compiler_options& options);
void compile_putenv(ASM::asmcode& code, const compiler_options& options);
void compile_eval(ASM::asmcode& code, const compiler_options& options);
void compile_load(ASM::asmcode& code, const compiler_options& options);
void compile_error(ASM::asmcode& code, const compiler_options& options);
void compile_is_eq(ASM::asmcode& code, const compiler_options& options);
void compile_is_eqv(ASM::asmcode& code, const compiler_options& options);
void compile_is_eqv_structurally(ASM::asmcode& code, const compiler_options& options);
void compile_is_equal(ASM::asmcode& code, const compiler_options& options);
void compile_add(ASM::asmcode& code, const compiler_options& options);
void compile_sub(ASM::asmcode& code, const compiler_options& options);
void compile_mul(ASM::asmcode& code, const compiler_options& options);
void compile_div(ASM::asmcode& code, const compiler_options& options);
void compile_max(ASM::asmcode& code, const compiler_options& options);
void compile_min(ASM::asmcode& code, const compiler_options& options);
void compile_equal(ASM::asmcode& code, const compiler_options& options);
void compile_not_equal(ASM::asmcode& code, const compiler_options& options);
void compile_less(ASM::asmcode& code, const compiler_options& options);
void compile_leq(ASM::asmcode& code, const compiler_options& options);
void compile_geq(ASM::asmcode& code, const compiler_options& options);
void compile_greater(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_and(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_not(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_or(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_xor(ASM::asmcode& code, const compiler_options& options);

void compile_make_promise(ASM::asmcode& code, const compiler_options& options);
void compile_compare_strings(ASM::asmcode& code, const compiler_options& options);
void compile_compare_strings_ci(ASM::asmcode& code, const compiler_options& options);
void compile_fx_equal(ASM::asmcode& code, const compiler_options& options);
void compile_fx_less(ASM::asmcode& code, const compiler_options& options);
void compile_fx_greater(ASM::asmcode& code, const compiler_options& options);
void compile_fx_leq(ASM::asmcode& code, const compiler_options& options);
void compile_fx_geq(ASM::asmcode& code, const compiler_options& options);
void compile_fx_add(ASM::asmcode& code, const compiler_options& options);
void compile_fx_sub(ASM::asmcode& code, const compiler_options& options);
void compile_fx_mul(ASM::asmcode& code, const compiler_options& options);
void compile_fx_div(ASM::asmcode& code, const compiler_options& options);
void compile_fx_add1(ASM::asmcode& code, const compiler_options& options);
void compile_fx_sub1(ASM::asmcode& code, const compiler_options& options);
void compile_fx_is_zero(ASM::asmcode& code, const compiler_options& options);
void compile_char_equal(ASM::asmcode& code, const compiler_options& options);
void compile_char_less(ASM::asmcode& code, const compiler_options& options);
void compile_char_greater(ASM::asmcode& code, const compiler_options& options);
void compile_char_leq(ASM::asmcode& code, const compiler_options& options);
void compile_char_geq(ASM::asmcode& code, const compiler_options& options);
void compile_char_to_fixnum(ASM::asmcode& code, const compiler_options& options);
void compile_fixnum_to_char(ASM::asmcode& code, const compiler_options& options);
void compile_closure(ASM::asmcode& code, const compiler_options& options);
void compile_closure_ref(ASM::asmcode& code, const compiler_options& options);
void compile_make_vector(ASM::asmcode& code, const compiler_options& options);
void compile_vector(ASM::asmcode& code, const compiler_options& options);
void compile_vector_length(ASM::asmcode& code, const compiler_options& options);
void compile_slot_ref(ASM::asmcode& code, const compiler_options& options);
void compile_slot_set(ASM::asmcode& code, const compiler_options& options);
void compile_vector_ref(ASM::asmcode& code, const compiler_options& options);
void compile_vector_set(ASM::asmcode& code, const compiler_options& options);
void compile_allocate_symbol(ASM::asmcode& code, const compiler_options& options);
void compile_make_string(ASM::asmcode& code, const compiler_options& options);
void compile_string(ASM::asmcode& code, const compiler_options& options);
void compile_string_length(ASM::asmcode& code, const compiler_options& options);
void compile_string_ref(ASM::asmcode& code, const compiler_options& options);
void compile_string_set(ASM::asmcode& code, const compiler_options& options);
void compile_list(ASM::asmcode& code, const compiler_options& options);
void compile_length(ASM::asmcode& code, const compiler_options& options);
void compile_set_car(ASM::asmcode& code, const compiler_options& options);
void compile_set_cdr(ASM::asmcode& code, const compiler_options& options);
void compile_make_port(ASM::asmcode& code, const compiler_options& options);
void compile_is_port(ASM::asmcode& code, const compiler_options& options);
void compile_is_input_port(ASM::asmcode& code, const compiler_options& options);
void compile_is_output_port(ASM::asmcode& code, const compiler_options& options);
void compile_read_char(ASM::asmcode& code, const compiler_options& options);
void compile_peek_char(ASM::asmcode& code, const compiler_options& options);
void compile_write_char(ASM::asmcode& code, const compiler_options& options);
void compile_write_string(ASM::asmcode& code, const compiler_options& options);
void compile_flush_output_port(ASM::asmcode& code, const compiler_options& options);
void compile_open_file(ASM::asmcode& code, const compiler_options& options);
void compile_close_file(ASM::asmcode& code, const compiler_options& options);
void compile_is_eof_object(ASM::asmcode& code, const compiler_options& options);
void compile_num2str(ASM::asmcode& code, const compiler_options& options);
void compile_str2num(ASM::asmcode& code, const compiler_options& options);
void compile_string_fill(ASM::asmcode& code, const compiler_options& options);
void compile_vector_fill(ASM::asmcode& code, const compiler_options& options);
void compile_substring(ASM::asmcode& code, const compiler_options& options);
void compile_string_append1(ASM::asmcode& code, const compiler_options& options);
void compile_string_hash(ASM::asmcode& code, const compiler_options& options);

void compile_ieee754_sign(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_exponent(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_mantissa(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_sin(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_cos(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_tan(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_asin(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_acos(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_atan1(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_log(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_round(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_truncate(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_sqrt(ASM::asmcode& code, const compiler_options& options);
void compile_ieee754_pi(ASM::asmcode& code, const compiler_options& options);
void compile_fixnum_expt(ASM::asmcode& code, const compiler_options& options);
void compile_flonum_expt(ASM::asmcode& code, const compiler_options& options);

void compile_not(ASM::asmcode& code, const compiler_options& options);
void compile_is_fixnum(ASM::asmcode& code, const compiler_options& options);
void compile_is_flonum(ASM::asmcode& code, const compiler_options& options);
void compile_is_nil(ASM::asmcode& code, const compiler_options& options);
void compile_is_vector(ASM::asmcode& code, const compiler_options& options);
void compile_is_pair(ASM::asmcode& code, const compiler_options& options);
void compile_is_string(ASM::asmcode& code, const compiler_options& options);
void compile_is_closure(ASM::asmcode& code, const compiler_options& options);
void compile_is_procedure(ASM::asmcode& code, const compiler_options&);
void compile_is_symbol(ASM::asmcode& code, const compiler_options& options);
void compile_is_promise(ASM::asmcode& code, const compiler_options& options);
void compile_is_zero(ASM::asmcode& code, const compiler_options& options);
void compile_is_char(ASM::asmcode& code, const compiler_options& options);
void compile_is_boolean(ASM::asmcode& code, const compiler_options& options);
void compile_member(ASM::asmcode& code, const compiler_options& options);
void compile_memv(ASM::asmcode& code, const compiler_options& options);
void compile_memq(ASM::asmcode& code, const compiler_options& ops);
void compile_assoc(ASM::asmcode& code, const compiler_options& options);
void compile_assv(ASM::asmcode& code, const compiler_options& options);
void compile_assq(ASM::asmcode& code, const compiler_options& ops);
void compile_apply(ASM::asmcode& code, const compiler_options& options);
void compile_cons(ASM::asmcode& code, const compiler_options& options);
void compile_car(ASM::asmcode& code, const compiler_options& options);
void compile_cdr(ASM::asmcode& code, const compiler_options& options);
void compile_reclaim_garbage(ASM::asmcode& code, const compiler_options& options);
void compile_reclaim(ASM::asmcode& code, const compiler_options& options);
void compile_arithmetic_shift(ASM::asmcode& code, const compiler_options& options);
void compile_quotient(ASM::asmcode& code, const compiler_options& options);
void compile_remainder(ASM::asmcode& code, const compiler_options& options);
void compile_fixnum_to_flonum(ASM::asmcode& code, const compiler_options& options);
void compile_flonum_to_fixnum(ASM::asmcode& code, const compiler_options& options);
void compile_string_copy(ASM::asmcode& code, const compiler_options& options);
void compile_symbol_to_string(ASM::asmcode& code, const compiler_options& options);
void compile_undefined(ASM::asmcode& code, const compiler_options& options);
void compile_skiwi_quiet_undefined(ASM::asmcode& code, const compiler_options& options);

void compile_member_cmp_eq(ASM::asmcode& code, const compiler_options&);
void compile_member_cmp_eqv(ASM::asmcode& code, const compiler_options&);
void compile_member_cmp_equal(ASM::asmcode& code, const compiler_options&);
void compile_assoc_cmp_eq(ASM::asmcode& code, const compiler_options&);
void compile_assoc_cmp_eqv(ASM::asmcode& code, const compiler_options&);
void compile_assoc_cmp_equal(ASM::asmcode& code, const compiler_options&);
void compile_structurally_equal(ASM::asmcode& code, const compiler_options&, const std::string& label_name);
void compile_recursively_equal(ASM::asmcode& code, const compiler_options&, const std::string& label_name);
void compile_mark(ASM::asmcode& code, const compiler_options&);
void compile_not_equal_2(ASM::asmcode& code, const compiler_options& ops);
void compile_equal_2(ASM::asmcode& code, const compiler_options& options);
void compile_less_2(ASM::asmcode& code, const compiler_options& options);
void compile_leq_2(ASM::asmcode& code, const compiler_options& options);
void compile_geq_2(ASM::asmcode& code, const compiler_options& options);
void compile_greater_2(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_and_2(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_or_2(ASM::asmcode& code, const compiler_options& options);
void compile_bitwise_xor_2(ASM::asmcode& code, const compiler_options& options);
void compile_add_2(ASM::asmcode& code, const compiler_options& options);
void compile_subtract_2(ASM::asmcode& code, const compiler_options& options);
void compile_multiply_2(ASM::asmcode& code, const compiler_options& options);
void compile_divide_2(ASM::asmcode& code, const compiler_options& options);
void compile_max_2(ASM::asmcode& code, const compiler_options& options);
void compile_min_2(ASM::asmcode& code, const compiler_options& options);
void compile_fold_binary(ASM::asmcode& code, const compiler_options& options);
void compile_pairwise_compare(ASM::asmcode& code, const compiler_options& options);
void compile_apply_fake_cps_identity(ASM::asmcode& code, const compiler_options& options);

SKIWI_END
