#include "inlines.h"
#include "context.h"
#include "context_defs.h"
#include "asm_aux.h"
#include "globals.h"

SKIWI_BEGIN

void save_to_local(asmcode& code, uint64_t pos)
  {
  code.add(asmcode::MOV, asmcode::R15, LOCAL);
  code.add(asmcode::MOV, asmcode::MEM_R15, CELLS(pos), asmcode::RAX);
  }

void save_to_local(asmcode& code, uint64_t pos, asmcode::operand source, asmcode::operand free_reg)
  {
  code.add(asmcode::MOV, free_reg, LOCAL);
  code.add(asmcode::MOV, get_mem_operand(free_reg), CELLS(pos), source);
  }

void load_local(asmcode& code, uint64_t pos)
  {
  code.add(asmcode::MOV, asmcode::R15, LOCAL);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_R15, CELLS(pos));
  }

void load_local(asmcode& code, uint64_t pos, asmcode::operand target, asmcode::operand free_reg)
  {
  code.add(asmcode::MOV, free_reg, LOCAL);
  code.add(asmcode::MOV, target, get_mem_operand(free_reg), CELLS(pos));
  }

void fix2int(asmcode& code, asmcode::operand reg)
  {
  code.add(asmcode::SAR, reg, asmcode::NUMBER, 1);
  }

void int2fix(asmcode& code, asmcode::operand reg)
  {
  code.add(asmcode::SHL, reg, asmcode::NUMBER, 1);
  }

void inline_is_fixnum(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::TEST, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_is_flonum(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_flonum(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_pair(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_pair(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_vector(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_vector(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_port(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_input_port(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_output_port(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 8); // toggle 4-th bit to switch from #t to #f or vice versa
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_string(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_string(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_symbol(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_symbol(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_promise(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_promise(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_closure(asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_nil(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, nil);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_is_eof_object(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, eof_tag);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_is_procedure(asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  auto check_closure = label_to_string(label++);
  auto lab_false = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::RAX);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, procedure_mask);
  code.add(asmcode::CMP, asmcode::RBX, asmcode::NUMBER, procedure_tag);
  code.add(asmcode::JNES, check_closure);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, check_closure);
  jump_short_if_arg_is_not_block(code, asmcode::RAX, asmcode::RBX, lab_false);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, asmcode::RAX, asmcode::RAX, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_t);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, lab_false);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::LABEL, done);
  }

void inline_is_boolean(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::AL, asmcode::NUMBER, 247);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_is_char(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::AL, asmcode::NUMBER, char_tag);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_min(asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::JLS, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::LABEL, done);
  }

void inline_fl_min(asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JBS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_max(asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::JGS, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::LABEL, done);
  }

void inline_fl_max(asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::UCOMISD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::JAS, done);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_add1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::ADD, asmcode::RAX, asmcode::NUMBER, 2);
  }

void inline_fl_add1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  double d = 1.0;
  code.add(asmcode::MOV, asmcode::RBX, asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_sub1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SUB, asmcode::RAX, asmcode::NUMBER, 2);
  }

void inline_fl_sub1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  double d = 1.0;
  code.add(asmcode::MOV, asmcode::RBX, asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RBX);
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_is_zero(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::TEST, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_is_zero(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RAX);
  code.add(asmcode::MOVQ, asmcode::XMM1, asmcode::RAX);
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_add(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::ADD, asmcode::RAX, asmcode::RBX);
  }

void inline_fl_add(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::ADDSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_sub(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SUB, asmcode::RAX, asmcode::RBX);
  }

void inline_fl_sub(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::SUBSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_mul(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::R15, asmcode::RDX);
  code.add(asmcode::IMUL, asmcode::RBX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  }

void inline_fl_mul(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::MULSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_fx_div(asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::RBX);
    code.add(asmcode::JES, div_by_zero);
    }
  code.add(asmcode::MOV, asmcode::R15, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RBX);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(asmcode::LABEL, done);
    }
  }

void inline_fl_div(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::DIVSD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVSD, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  }

void inline_eq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_less(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETL, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_less(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_leq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETLE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_leq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_greater(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETG, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_greater(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLTPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_not_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_not_equal(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPEQPD, asmcode::XMM0, asmcode::XMM1);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fx_geq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SETGE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_fl_geq(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVSD, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVSD, asmcode::XMM1, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMPLEPD, asmcode::XMM1, asmcode::XMM0);
  code.add(asmcode::MOVMSKPD, asmcode::RAX, asmcode::XMM1);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_cons(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(1), asmcode::RAX);
  code.add(asmcode::MOV, MEM_ALLOC, CELLS(2), asmcode::RBX);
  uint64_t header = make_block_header(2, T_PAIR);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOV, asmcode::RAX, ALLOC);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(3));
  }

void inline_car(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  }

void inline_cdr(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(2));
  }

void inline_set_car(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(1), asmcode::RBX);
  }

void inline_set_cdr(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::MEM_RAX, CELLS(2), asmcode::RBX);
  }

void inline_not(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::CMP, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::SETE, asmcode::AL);
  code.add(asmcode::MOVZX, asmcode::RAX, asmcode::AL);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 3);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, bool_f);
  }

void inline_memq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    }
  std::string fail = label_to_string(label++);
  std::string success = label_to_string(label++);
  std::string repeat = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::RBX, asmcode::NUMBER, nil);
  code.add(asmcode::JES, fail);

  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, asmcode::RBX, asmcode::R15, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, asmcode::RBX, asmcode::R15, error);
    }
  // get car
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RBX, CELLS(1));
  code.add(asmcode::CMP, asmcode::RAX, asmcode::R15);
  code.add(asmcode::JES, success);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_RBX, CELLS(2));
  code.add(asmcode::JMPS, repeat);

  code.add(asmcode::LABEL, fail);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, success);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag); // add tag again  

  if (ops.safe_primitives)
    {
    code.add(asmcode::JMPS, done);
    error_label(code, error, re_memq_contract_violation);
    }

  code.add(asmcode::LABEL, done);
  }

void inline_assq(asmcode& code, const compiler_options& ops)
  {
  std::string error;
  if (ops.safe_primitives)
    {
    error = label_to_string(label++);
    }
  std::string fail = label_to_string(label++);
  std::string success = label_to_string(label++);
  std::string repeat = label_to_string(label++);
  std::string skip_nil = label_to_string(label++);
  std::string done = label_to_string(label++);
  code.add(asmcode::LABEL, repeat);
  code.add(asmcode::CMP, asmcode::RBX, asmcode::NUMBER, nil);
  code.add(asmcode::JE, fail);

  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, asmcode::RBX, asmcode::R15, error);
    }
  code.add(asmcode::AND, asmcode::RBX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, asmcode::RBX, asmcode::R15, error);
    }
  // get car
  code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RBX, CELLS(1));
  // test for nil
  code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, nil);
  code.add(asmcode::JES, skip_nil);

  if (ops.safe_primitives)
    {
    code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, block_mask);
    code.add(asmcode::CMP, asmcode::R15, asmcode::NUMBER, block_tag);
    code.add(asmcode::JNES, error);
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RBX, CELLS(1));
    }
  code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, asmcode::R15, asmcode::R15, error);
    code.add(asmcode::MOV, asmcode::R15, asmcode::MEM_RBX, CELLS(1));
    code.add(asmcode::AND, asmcode::R15, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    }

  code.add(asmcode::CMP, asmcode::RAX, asmcode::MEM_R15, CELLS(1));
  code.add(asmcode::JES, success);
  code.add(asmcode::LABEL, skip_nil);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::MEM_RBX, CELLS(2));
  code.add(asmcode::JMPS, repeat);

  code.add(asmcode::LABEL, fail);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, bool_f);
  code.add(asmcode::JMPS, done);

  code.add(asmcode::LABEL, success);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::R15);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, block_tag); // add tag again  

  if (ops.safe_primitives)
    {
    code.add(asmcode::JMPS, done);
    error_label(code, error, re_assq_contract_violation);
    }

  code.add(asmcode::LABEL, done);

  }

void inline_ieee754_sign(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 63);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_ieee754_exponent(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 52); // mantissa is 52
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0x7ff); // remove sign bit
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_ieee754_mantissa(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOV, asmcode::RBX, asmcode::NUMBER, 0x000fffffffffffff);
  code.add(asmcode::AND, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_ieee754_fxsin(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FSIN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxcos(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FCOS);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxtan(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FPTAN);
  code.add(asmcode::FSTP, asmcode::ST0);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxasin(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxacos(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FXCH, asmcode::ST1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxatan1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxlog(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::FLDLN2);

  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FYL2X);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxround(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::PUSH, asmcode::RAX);
  code.add(asmcode::FILD, asmcode::MEM_RSP);
  code.add(asmcode::POP, asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FRNDINT);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fxtruncate(asmcode&, const compiler_options&)
  {
  //do nothing
  }

void inline_ieee754_fxsqrt(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  code.add(asmcode::SQRTPD, asmcode::XMM0, asmcode::XMM0);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flsin(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FSIN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flcos(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FCOS);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fltan(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FPTAN);
  code.add(asmcode::FSTP, asmcode::ST0);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flasin(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flacos(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD, asmcode::ST0);
  code.add(asmcode::FMUL, asmcode::ST0, asmcode::ST0);
  code.add(asmcode::FLD1);
  code.add(asmcode::FSUBRP);
  code.add(asmcode::FSQRT);
  code.add(asmcode::FXCH, asmcode::ST1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flatan1(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLD1);
  code.add(asmcode::FPATAN);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fllog(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::FLDLN2);

  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);

  code.add(asmcode::FYL2X);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_flround(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::FLD, asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FRNDINT);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_fltruncate(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::CVTTSD2SI, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_ieee754_flsqrt(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::SQRTPD, asmcode::XMM0, asmcode::XMM0);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_ieee754_pi(asmcode& code, const compiler_options&)
  {
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::FLDPI);
  code.add(asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_bitwise_and(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::RBX);
  }

void inline_bitwise_not(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::XOR, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFFF);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFFE);
  }

void inline_bitwise_or(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::OR, asmcode::RAX, asmcode::RBX);
  }

void inline_bitwise_xor(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::XOR, asmcode::RAX, asmcode::RBX);
  }

void inline_char_to_fixnum(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SHR, asmcode::RAX, asmcode::NUMBER, 7);
  }

void inline_fixnum_to_char(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 7);
  code.add(asmcode::OR, asmcode::RAX, asmcode::NUMBER, char_tag);
  }

void inline_vector_length(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX);
  code.add(asmcode::MOV, asmcode::RBX, asmcode::NUMBER, block_size_mask);
  code.add(asmcode::AND, asmcode::RAX, asmcode::RBX);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_flonum_to_fixnum(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RAX, CELLS(1));
  code.add(asmcode::MOVQ, asmcode::XMM0, asmcode::RAX);
  code.add(asmcode::CVTTSD2SI, asmcode::RAX, asmcode::XMM0);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  }

void inline_fixnum_to_flonum(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::CVTSI2SD, asmcode::XMM0, asmcode::RAX);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(asmcode::MOV, asmcode::RBX, ALLOC);
  code.add(asmcode::OR, asmcode::RBX, asmcode::NUMBER, block_tag);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, header);
  code.add(asmcode::MOV, MEM_ALLOC, asmcode::RAX);
  code.add(asmcode::MOVQ, MEM_ALLOC, CELLS(1), asmcode::XMM0);
  code.add(asmcode::ADD, ALLOC, asmcode::NUMBER, CELLS(2));
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RBX);
  }

void inline_undefined(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, undefined);
  }

void inline_quiet_undefined(asmcode& code, const compiler_options&)
  {
  code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, quiet_undefined);
  }

void inline_arithmetic_shift(asmcode& code, const compiler_options&)
  {
  auto shift_right = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(asmcode::MOV, asmcode::R15, asmcode::RCX);
  code.add(asmcode::CMP, asmcode::RBX, asmcode::NUMBER, 0);
  code.add(asmcode::JLS, shift_right);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RBX);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::CL);
  code.add(asmcode::JMPS, done);
  code.add(asmcode::LABEL, shift_right);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::NEG, asmcode::RBX);
  code.add(asmcode::SHR, asmcode::RBX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::RBX);
  code.add(asmcode::SAR, asmcode::RAX, asmcode::CL);
  code.add(asmcode::SAL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::LABEL, done);
  code.add(asmcode::MOV, asmcode::RCX, asmcode::R15);
  }

void inline_quotient(asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::RBX);
    code.add(asmcode::JES, div_by_zero);
    }
  code.add(asmcode::MOV, asmcode::R15, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RBX);
  code.add(asmcode::SHL, asmcode::RAX, asmcode::NUMBER, 1);
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(asmcode::LABEL, done);
    }
  }

void inline_remainder(asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(asmcode::TEST, asmcode::RBX, asmcode::RBX);
    code.add(asmcode::JES, div_by_zero);
    }
  code.add(asmcode::MOV, asmcode::R15, asmcode::RDX);
  code.add(asmcode::XOR, asmcode::RDX, asmcode::RDX);
  code.add(asmcode::CQO);
  code.add(asmcode::IDIV, asmcode::RBX);
  code.add(asmcode::MOV, asmcode::RAX, asmcode::RDX);
  code.add(asmcode::AND, asmcode::RAX, asmcode::NUMBER, ~((uint64_t)1));
  code.add(asmcode::MOV, asmcode::RDX, asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(asmcode::LABEL, done);
    }
  }

SKIWI_END
