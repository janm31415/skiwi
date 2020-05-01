#pragma once

#include <asm/asmcode.h>
#include "namespace.h"

#include "compiler_options.h"

SKIWI_BEGIN

// destroys R15
void save_to_local(ASM::asmcode& code, uint64_t pos);

// destroys R15
void load_local(ASM::asmcode& code, uint64_t pos);

void save_to_local(ASM::asmcode& code, uint64_t pos, ASM::asmcode::operand source, ASM::asmcode::operand free_reg);
void load_local(ASM::asmcode& code, uint64_t pos, ASM::asmcode::operand target, ASM::asmcode::operand free_reg);

void fix2int(ASM::asmcode& code, ASM::asmcode::operand reg);
void int2fix(ASM::asmcode& code, ASM::asmcode::operand reg);

void inline_is_fixnum(ASM::asmcode& code, const compiler_options& options);
void inline_is_flonum(ASM::asmcode& code, const compiler_options& options);
void inline_is_pair(ASM::asmcode& code, const compiler_options& options);
void inline_is_vector(ASM::asmcode& code, const compiler_options& options);
void inline_is_string(ASM::asmcode& code, const compiler_options& options);
void inline_is_symbol(ASM::asmcode& code, const compiler_options& options);
void inline_is_promise(ASM::asmcode& code, const compiler_options& options);
void inline_is_closure(ASM::asmcode& code, const compiler_options& options);
void inline_is_nil(ASM::asmcode& code, const compiler_options& options);
void inline_is_eof_object(ASM::asmcode& code, const compiler_options& options);
void inline_is_procedure(ASM::asmcode& code, const compiler_options& options);
void inline_is_boolean(ASM::asmcode& code, const compiler_options& options);
void inline_is_char(ASM::asmcode& code, const compiler_options& options);
void inline_is_port(ASM::asmcode& code, const compiler_options& options);
void inline_is_input_port(ASM::asmcode& code, const compiler_options& options);
void inline_is_output_port(ASM::asmcode& code, const compiler_options& options);

void inline_eq(ASM::asmcode& code, const compiler_options& options);
void inline_fx_add(ASM::asmcode& code, const compiler_options& options);
void inline_fl_add(ASM::asmcode& code, const compiler_options& options);
void inline_fx_sub(ASM::asmcode& code, const compiler_options& options);
void inline_fl_sub(ASM::asmcode& code, const compiler_options& options);
void inline_fx_mul(ASM::asmcode& code, const compiler_options& options);
void inline_fl_mul(ASM::asmcode& code, const compiler_options& options);
void inline_fx_div(ASM::asmcode& code, const compiler_options& options);
void inline_fl_div(ASM::asmcode& code, const compiler_options& options);

void inline_fx_add1(ASM::asmcode& code, const compiler_options& options);
void inline_fl_add1(ASM::asmcode& code, const compiler_options& options);
void inline_fx_sub1(ASM::asmcode& code, const compiler_options& options);
void inline_fl_sub1(ASM::asmcode& code, const compiler_options& options);
void inline_fx_is_zero(ASM::asmcode& code, const compiler_options& options);
void inline_fl_is_zero(ASM::asmcode& code, const compiler_options& options);
void inline_fx_min(ASM::asmcode& code, const compiler_options& options);
void inline_fl_min(ASM::asmcode& code, const compiler_options& options);
void inline_fx_max(ASM::asmcode& code, const compiler_options& options);
void inline_fl_max(ASM::asmcode& code, const compiler_options& options);

void inline_fx_less(ASM::asmcode& code, const compiler_options& options);
void inline_fl_less(ASM::asmcode& code, const compiler_options& options);
void inline_fx_leq(ASM::asmcode& code, const compiler_options& options);
void inline_fl_leq(ASM::asmcode& code, const compiler_options& options);
void inline_fx_greater(ASM::asmcode& code, const compiler_options& options);
void inline_fl_greater(ASM::asmcode& code, const compiler_options& options);
void inline_fx_geq(ASM::asmcode& code, const compiler_options& options);
void inline_fl_geq(ASM::asmcode& code, const compiler_options& options);
void inline_fx_equal(ASM::asmcode& code, const compiler_options& options);
void inline_fl_equal(ASM::asmcode& code, const compiler_options& options);
void inline_fx_not_equal(ASM::asmcode& code, const compiler_options& options);
void inline_fl_not_equal(ASM::asmcode& code, const compiler_options& options);

void inline_cons(ASM::asmcode& code, const compiler_options& options);
void inline_car(ASM::asmcode& code, const compiler_options& options);
void inline_cdr(ASM::asmcode& code, const compiler_options& options);
void inline_set_car(ASM::asmcode& code, const compiler_options& options);
void inline_set_cdr(ASM::asmcode& code, const compiler_options& options);

void inline_not(ASM::asmcode& code, const compiler_options& options);
void inline_memq(ASM::asmcode& code, const compiler_options& options);
void inline_assq(ASM::asmcode& code, const compiler_options& options);

void inline_ieee754_sign(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_exponent(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_mantissa(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxsin(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxcos(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxtan(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxasin(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxacos(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxatan1(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxlog(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxround(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxtruncate(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fxsqrt(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flsin(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flcos(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fltan(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flasin(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flacos(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flatan1(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fllog(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flround(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_fltruncate(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_flsqrt(ASM::asmcode& code, const compiler_options& options);
void inline_ieee754_pi(ASM::asmcode& code, const compiler_options& options);

void inline_bitwise_and(ASM::asmcode& code, const compiler_options& options);
void inline_bitwise_not(ASM::asmcode& code, const compiler_options& options);
void inline_bitwise_or(ASM::asmcode& code, const compiler_options& options);
void inline_bitwise_xor(ASM::asmcode& code, const compiler_options& options);

void inline_char_to_fixnum(ASM::asmcode& code, const compiler_options& options);
void inline_fixnum_to_char(ASM::asmcode& code, const compiler_options& options);
void inline_vector_length(ASM::asmcode& code, const compiler_options& options);
void inline_flonum_to_fixnum(ASM::asmcode& code, const compiler_options& options);
void inline_fixnum_to_flonum(ASM::asmcode& code, const compiler_options& options);

void inline_undefined(ASM::asmcode& code, const compiler_options& options);
void inline_quiet_undefined(ASM::asmcode& code, const compiler_options& options);

void inline_arithmetic_shift(ASM::asmcode& code, const compiler_options& options);
void inline_quotient(ASM::asmcode& code, const compiler_options& options);
void inline_remainder(ASM::asmcode& code, const compiler_options& options);

SKIWI_END
