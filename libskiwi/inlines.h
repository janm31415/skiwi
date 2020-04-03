#pragma once

#include <asm/asmcode.h>
#include <asm/namespace.h>

#include "compiler_options.h"

SKIWI_BEGIN

// destroys R15
void save_to_local(asmcode& code, uint64_t pos);

// destroys R15
void load_local(asmcode& code, uint64_t pos);

void save_to_local(asmcode& code, uint64_t pos, asmcode::operand source, asmcode::operand free_reg);
void load_local(asmcode& code, uint64_t pos, asmcode::operand target, asmcode::operand free_reg);

void fix2int(asmcode& code, asmcode::operand reg);
void int2fix(asmcode& code, asmcode::operand reg);

void inline_is_fixnum(asmcode& code, const compiler_options& options);
void inline_is_flonum(asmcode& code, const compiler_options& options);
void inline_is_pair(asmcode& code, const compiler_options& options);
void inline_is_vector(asmcode& code, const compiler_options& options);
void inline_is_string(asmcode& code, const compiler_options& options);
void inline_is_symbol(asmcode& code, const compiler_options& options);
void inline_is_promise(asmcode& code, const compiler_options& options);
void inline_is_closure(asmcode& code, const compiler_options& options);
void inline_is_nil(asmcode& code, const compiler_options& options);
void inline_is_eof_object(asmcode& code, const compiler_options& options);
void inline_is_procedure(asmcode& code, const compiler_options& options);
void inline_is_boolean(asmcode& code, const compiler_options& options);
void inline_is_char(asmcode& code, const compiler_options& options);
void inline_is_port(asmcode& code, const compiler_options& options);
void inline_is_input_port(asmcode& code, const compiler_options& options);
void inline_is_output_port(asmcode& code, const compiler_options& options);

void inline_eq(asmcode& code, const compiler_options& options);
void inline_fx_add(asmcode& code, const compiler_options& options);
void inline_fl_add(asmcode& code, const compiler_options& options);
void inline_fx_sub(asmcode& code, const compiler_options& options);
void inline_fl_sub(asmcode& code, const compiler_options& options);
void inline_fx_mul(asmcode& code, const compiler_options& options);
void inline_fl_mul(asmcode& code, const compiler_options& options);
void inline_fx_div(asmcode& code, const compiler_options& options);
void inline_fl_div(asmcode& code, const compiler_options& options);

void inline_fx_add1(asmcode& code, const compiler_options& options);
void inline_fl_add1(asmcode& code, const compiler_options& options);
void inline_fx_sub1(asmcode& code, const compiler_options& options);
void inline_fl_sub1(asmcode& code, const compiler_options& options);
void inline_fx_is_zero(asmcode& code, const compiler_options& options);
void inline_fl_is_zero(asmcode& code, const compiler_options& options);
void inline_fx_min(asmcode& code, const compiler_options& options);
void inline_fl_min(asmcode& code, const compiler_options& options);
void inline_fx_max(asmcode& code, const compiler_options& options);
void inline_fl_max(asmcode& code, const compiler_options& options);

void inline_fx_less(asmcode& code, const compiler_options& options);
void inline_fl_less(asmcode& code, const compiler_options& options);
void inline_fx_leq(asmcode& code, const compiler_options& options);
void inline_fl_leq(asmcode& code, const compiler_options& options);
void inline_fx_greater(asmcode& code, const compiler_options& options);
void inline_fl_greater(asmcode& code, const compiler_options& options);
void inline_fx_geq(asmcode& code, const compiler_options& options);
void inline_fl_geq(asmcode& code, const compiler_options& options);
void inline_fx_equal(asmcode& code, const compiler_options& options);
void inline_fl_equal(asmcode& code, const compiler_options& options);
void inline_fx_not_equal(asmcode& code, const compiler_options& options);
void inline_fl_not_equal(asmcode& code, const compiler_options& options);

void inline_cons(asmcode& code, const compiler_options& options);
void inline_car(asmcode& code, const compiler_options& options);
void inline_cdr(asmcode& code, const compiler_options& options);
void inline_set_car(asmcode& code, const compiler_options& options);
void inline_set_cdr(asmcode& code, const compiler_options& options);

void inline_not(asmcode& code, const compiler_options& options);
void inline_memq(asmcode& code, const compiler_options& options);
void inline_assq(asmcode& code, const compiler_options& options);

void inline_ieee754_sign(asmcode& code, const compiler_options& options);
void inline_ieee754_exponent(asmcode& code, const compiler_options& options);
void inline_ieee754_mantissa(asmcode& code, const compiler_options& options);
void inline_ieee754_fxsin(asmcode& code, const compiler_options& options);
void inline_ieee754_fxcos(asmcode& code, const compiler_options& options);
void inline_ieee754_fxtan(asmcode& code, const compiler_options& options);
void inline_ieee754_fxasin(asmcode& code, const compiler_options& options);
void inline_ieee754_fxacos(asmcode& code, const compiler_options& options);
void inline_ieee754_fxatan1(asmcode& code, const compiler_options& options);
void inline_ieee754_fxlog(asmcode& code, const compiler_options& options);
void inline_ieee754_fxround(asmcode& code, const compiler_options& options);
void inline_ieee754_fxtruncate(asmcode& code, const compiler_options& options);
void inline_ieee754_fxsqrt(asmcode& code, const compiler_options& options);
void inline_ieee754_flsin(asmcode& code, const compiler_options& options);
void inline_ieee754_flcos(asmcode& code, const compiler_options& options);
void inline_ieee754_fltan(asmcode& code, const compiler_options& options);
void inline_ieee754_flasin(asmcode& code, const compiler_options& options);
void inline_ieee754_flacos(asmcode& code, const compiler_options& options);
void inline_ieee754_flatan1(asmcode& code, const compiler_options& options);
void inline_ieee754_fllog(asmcode& code, const compiler_options& options);
void inline_ieee754_flround(asmcode& code, const compiler_options& options);
void inline_ieee754_fltruncate(asmcode& code, const compiler_options& options);
void inline_ieee754_flsqrt(asmcode& code, const compiler_options& options);
void inline_ieee754_pi(asmcode& code, const compiler_options& options);

void inline_bitwise_and(asmcode& code, const compiler_options& options);
void inline_bitwise_not(asmcode& code, const compiler_options& options);
void inline_bitwise_or(asmcode& code, const compiler_options& options);
void inline_bitwise_xor(asmcode& code, const compiler_options& options);

void inline_char_to_fixnum(asmcode& code, const compiler_options& options);
void inline_fixnum_to_char(asmcode& code, const compiler_options& options);
void inline_vector_length(asmcode& code, const compiler_options& options);
void inline_flonum_to_fixnum(asmcode& code, const compiler_options& options);
void inline_fixnum_to_flonum(asmcode& code, const compiler_options& options);

void inline_undefined(asmcode& code, const compiler_options& options);
void inline_quiet_undefined(asmcode& code, const compiler_options& options);

void inline_arithmetic_shift(asmcode& code, const compiler_options& options);
void inline_quotient(asmcode& code, const compiler_options& options);
void inline_remainder(asmcode& code, const compiler_options& options);

SKIWI_END
