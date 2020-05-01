#include "inlines.h"
#include "context.h"
#include "context_defs.h"
#include "asm_aux.h"
#include "globals.h"

SKIWI_BEGIN

void save_to_local(ASM::asmcode& code, uint64_t pos)
  {
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, LOCAL);
  code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_R15, CELLS(pos), ASM::asmcode::RAX);
  }

void save_to_local(ASM::asmcode& code, uint64_t pos, ASM::asmcode::operand source, ASM::asmcode::operand free_reg)
  {
  code.add(ASM::asmcode::MOV, free_reg, LOCAL);
  code.add(ASM::asmcode::MOV, get_mem_operand(free_reg), CELLS(pos), source);
  }

void load_local(ASM::asmcode& code, uint64_t pos)
  {
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, LOCAL);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_R15, CELLS(pos));
  }

void load_local(ASM::asmcode& code, uint64_t pos, ASM::asmcode::operand target, ASM::asmcode::operand free_reg)
  {
  code.add(ASM::asmcode::MOV, free_reg, LOCAL);
  code.add(ASM::asmcode::MOV, target, get_mem_operand(free_reg), CELLS(pos));
  }

void fix2int(ASM::asmcode& code, ASM::asmcode::operand reg)
  {
  code.add(ASM::asmcode::SAR, reg, ASM::asmcode::NUMBER, 1);
  }

void int2fix(ASM::asmcode& code, ASM::asmcode::operand reg)
  {
  code.add(ASM::asmcode::SHL, reg, ASM::asmcode::NUMBER, 1);
  }

void inline_is_fixnum(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::TEST, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_is_flonum(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_flonum(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_pair(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_pair(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_vector(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_vector(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_port(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_input_port(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_output_port(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_port(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 8); // toggle 4-th bit to switch from #t to #f or vice versa
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_string(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_string(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_symbol(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_symbol(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_promise(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_promise(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_closure(ASM::asmcode& code, const compiler_options&)
  {
  auto lab_false = label_to_string(label++);
  auto done = label_to_string(label++);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_nil(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::NUMBER, nil);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_is_eof_object(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::NUMBER, eof_tag);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_is_procedure(ASM::asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  auto check_closure = label_to_string(label++);
  auto lab_false = label_to_string(label++);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::RAX);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, procedure_mask);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RBX, ASM::asmcode::NUMBER, procedure_tag);
  code.add(ASM::asmcode::JNES, check_closure);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, check_closure);
  jump_short_if_arg_is_not_block(code, ASM::asmcode::RAX, ASM::asmcode::RBX, lab_false);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  jump_short_if_arg_does_not_point_to_closure(code, ASM::asmcode::RAX, ASM::asmcode::RAX, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_t);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, lab_false);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_is_boolean(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::AL, ASM::asmcode::NUMBER, 247);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_is_char(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::AL, ASM::asmcode::NUMBER, char_tag);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_min(ASM::asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::JLS, done);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_fl_min(ASM::asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::UCOMISD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::JBS, done);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::LABEL, done);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_max(ASM::asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::JGS, done);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::LABEL, done);
  }

void inline_fl_max(ASM::asmcode& code, const compiler_options&)
  {
  auto done = label_to_string(label++);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::UCOMISD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::JAS, done);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::LABEL, done);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_add1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::ADD, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 2);
  }

void inline_fl_add1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  double d = 1.0;
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM1, ASM::asmcode::RBX);
  code.add(ASM::asmcode::ADDSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_sub1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SUB, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 2);
  }

void inline_fl_sub1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  double d = 1.0;
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&d)));
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM1, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SUBSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_is_zero(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::TEST, ASM::asmcode::RAX, ASM::asmcode::RAX);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_is_zero(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM1, ASM::asmcode::RAX);
  code.add(ASM::asmcode::CMPEQPD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_add(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::ADD, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_fl_add(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::ADDSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_sub(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SUB, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_fl_sub(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::SUBSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_mul(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::RDX);
  code.add(ASM::asmcode::IMUL, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RDX, ASM::asmcode::R15);
  }

void inline_fl_mul(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::MULSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_fx_div(ASM::asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(ASM::asmcode::TEST, ASM::asmcode::RBX, ASM::asmcode::RBX);
    code.add(ASM::asmcode::JES, div_by_zero);
    }
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::RDX);
  code.add(ASM::asmcode::XOR, ASM::asmcode::RDX, ASM::asmcode::RDX);
  code.add(ASM::asmcode::CQO);
  code.add(ASM::asmcode::IDIV, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SAL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RDX, ASM::asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(ASM::asmcode::LABEL, done);
    }
  }

void inline_fl_div(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::DIVSD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVSD, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  }

void inline_eq(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_less(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETL, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_less(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPLTPD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_leq(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETLE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_leq(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPLEPD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_greater(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETG, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_greater(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPLTPD, ASM::asmcode::XMM1, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_equal(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_equal(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPEQPD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_not_equal(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_not_equal(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPEQPD, ASM::asmcode::XMM0, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fx_geq(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SETGE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_fl_geq(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVSD, ASM::asmcode::XMM1, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMPLEPD, ASM::asmcode::XMM1, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::MOVMSKPD, ASM::asmcode::RAX, ASM::asmcode::XMM1);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_cons(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::MOV, MEM_ALLOC, CELLS(1), ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, CELLS(2), ASM::asmcode::RBX);
  uint64_t header = make_block_header(2, T_PAIR);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(3));
  }

void inline_car(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  }

void inline_cdr(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(2));
  }

void inline_set_car(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_RAX, CELLS(1), ASM::asmcode::RBX);
  }

void inline_set_cdr(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::MEM_RAX, CELLS(2), ASM::asmcode::RBX);
  }

void inline_not(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::SETE, ASM::asmcode::AL);
  code.add(ASM::asmcode::MOVZX, ASM::asmcode::RAX, ASM::asmcode::AL);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 3);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  }

void inline_memq(ASM::asmcode& code, const compiler_options& ops)
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
  code.add(ASM::asmcode::LABEL, repeat);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RBX, ASM::asmcode::NUMBER, nil);
  code.add(ASM::asmcode::JES, fail);

  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, ASM::asmcode::RBX, ASM::asmcode::R15, error);
    }
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, ASM::asmcode::RBX, ASM::asmcode::R15, error);
    }
  // get car
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::MEM_RBX, CELLS(1));
  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::R15);
  code.add(ASM::asmcode::JES, success);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::MEM_RBX, CELLS(2));
  code.add(ASM::asmcode::JMPS, repeat);

  code.add(ASM::asmcode::LABEL, fail);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::JMPS, done);

  code.add(ASM::asmcode::LABEL, success);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag); // add tag again  

  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::JMPS, done);
    error_label(code, error, re_memq_contract_violation);
    }

  code.add(ASM::asmcode::LABEL, done);
  }

void inline_assq(ASM::asmcode& code, const compiler_options& ops)
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
  code.add(ASM::asmcode::LABEL, repeat);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RBX, ASM::asmcode::NUMBER, nil);
  code.add(ASM::asmcode::JE, fail);

  if (ops.safe_primitives)
    {
    jump_short_if_arg_is_not_block(code, ASM::asmcode::RBX, ASM::asmcode::R15, error);
    }
  code.add(ASM::asmcode::AND, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, ASM::asmcode::RBX, ASM::asmcode::R15, error);
    }
  // get car
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::MEM_RBX, CELLS(1));
  // test for nil
  code.add(ASM::asmcode::CMP, ASM::asmcode::R15, ASM::asmcode::NUMBER, nil);
  code.add(ASM::asmcode::JES, skip_nil);

  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::AND, ASM::asmcode::R15, ASM::asmcode::NUMBER, block_mask);
    code.add(ASM::asmcode::CMP, ASM::asmcode::R15, ASM::asmcode::NUMBER, block_tag);
    code.add(ASM::asmcode::JNES, error);
    code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::MEM_RBX, CELLS(1));
    }
  code.add(ASM::asmcode::AND, ASM::asmcode::R15, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  if (ops.safe_primitives)
    {
    jump_short_if_arg_does_not_point_to_pair(code, ASM::asmcode::R15, ASM::asmcode::R15, error);
    code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::MEM_RBX, CELLS(1));
    code.add(ASM::asmcode::AND, ASM::asmcode::R15, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
    }

  code.add(ASM::asmcode::CMP, ASM::asmcode::RAX, ASM::asmcode::MEM_R15, CELLS(1));
  code.add(ASM::asmcode::JES, success);
  code.add(ASM::asmcode::LABEL, skip_nil);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::MEM_RBX, CELLS(2));
  code.add(ASM::asmcode::JMPS, repeat);

  code.add(ASM::asmcode::LABEL, fail);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, bool_f);
  code.add(ASM::asmcode::JMPS, done);

  code.add(ASM::asmcode::LABEL, success);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::R15);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, block_tag); // add tag again  

  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::JMPS, done);
    error_label(code, error, re_assq_contract_violation);
    }

  code.add(ASM::asmcode::LABEL, done);

  }

void inline_ieee754_sign(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::SHR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 63);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_ieee754_exponent(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 52); // mantissa is 52
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0x7ff); // remove sign bit
  code.add(ASM::asmcode::SAL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_ieee754_mantissa(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0x000fffffffffffff);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_ieee754_fxsin(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FSIN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxcos(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FCOS);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxtan(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FPTAN);
  code.add(ASM::asmcode::FSTP, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxasin(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);

  code.add(ASM::asmcode::FLD, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FMUL, ASM::asmcode::ST0, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FSUBRP);
  code.add(ASM::asmcode::FSQRT);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxacos(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FLD, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FMUL, ASM::asmcode::ST0, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FSUBRP);
  code.add(ASM::asmcode::FSQRT);
  code.add(ASM::asmcode::FXCH, ASM::asmcode::ST1);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxatan1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxlog(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::FLDLN2);

  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);

  code.add(ASM::asmcode::FYL2X);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxround(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::PUSH, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FILD, ASM::asmcode::MEM_RSP);
  code.add(ASM::asmcode::POP, ASM::asmcode::RAX);

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FRNDINT);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fxtruncate(ASM::asmcode&, const compiler_options&)
  {
  //do nothing
  }

void inline_ieee754_fxsqrt(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::CVTSI2SD, ASM::asmcode::XMM0, ASM::asmcode::RAX);
  code.add(ASM::asmcode::SQRTPD, ASM::asmcode::XMM0, ASM::asmcode::XMM0);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVQ, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flsin(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FSIN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flcos(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FCOS);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fltan(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FPTAN);
  code.add(ASM::asmcode::FSTP, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flasin(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);

  code.add(ASM::asmcode::FLD, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FMUL, ASM::asmcode::ST0, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FSUBRP);
  code.add(ASM::asmcode::FSQRT);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flacos(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FLD, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FMUL, ASM::asmcode::ST0, ASM::asmcode::ST0);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FSUBRP);
  code.add(ASM::asmcode::FSQRT);
  code.add(ASM::asmcode::FXCH, ASM::asmcode::ST1);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flatan1(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FLD1);
  code.add(ASM::asmcode::FPATAN);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fllog(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::FLDLN2);

  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);

  code.add(ASM::asmcode::FYL2X);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_flround(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::FLD, ASM::asmcode::MEM_RAX, CELLS(1));

  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FRNDINT);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_fltruncate(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::CVTTSD2SI, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::SAL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_ieee754_flsqrt(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM0, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::SQRTPD, ASM::asmcode::XMM0, ASM::asmcode::XMM0);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVQ, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_ieee754_pi(ASM::asmcode& code, const compiler_options&)
  {
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::FLDPI);
  code.add(ASM::asmcode::FSTP, MEM_ALLOC, CELLS(1));
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_bitwise_and(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_bitwise_not(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFFF);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFFE);
  }

void inline_bitwise_or(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_bitwise_xor(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::XOR, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_char_to_fixnum(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SHR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 7);
  }

void inline_fixnum_to_char(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 7);
  code.add(ASM::asmcode::OR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, char_tag);
  }

void inline_vector_length(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_size_mask);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_flonum_to_fixnum(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 0xFFFFFFFFFFFFFFF8);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::MEM_RAX, CELLS(1));
  code.add(ASM::asmcode::MOVQ, ASM::asmcode::XMM0, ASM::asmcode::RAX);
  code.add(ASM::asmcode::CVTTSD2SI, ASM::asmcode::RAX, ASM::asmcode::XMM0);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  }

void inline_fixnum_to_flonum(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::CVTSI2SD, ASM::asmcode::XMM0, ASM::asmcode::RAX);
  uint64_t header = make_block_header(1, T_FLONUM);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RBX, ALLOC);
  code.add(ASM::asmcode::OR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, block_tag);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, header);
  code.add(ASM::asmcode::MOV, MEM_ALLOC, ASM::asmcode::RAX);
  code.add(ASM::asmcode::MOVQ, MEM_ALLOC, CELLS(1), ASM::asmcode::XMM0);
  code.add(ASM::asmcode::ADD, ALLOC, ASM::asmcode::NUMBER, CELLS(2));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RBX);
  }

void inline_undefined(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, undefined);
  }

void inline_quiet_undefined(ASM::asmcode& code, const compiler_options&)
  {
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::NUMBER, quiet_undefined);
  }

void inline_arithmetic_shift(ASM::asmcode& code, const compiler_options&)
  {
  auto shift_right = label_to_string(label++);
  auto done = label_to_string(label++);
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::RCX);
  code.add(ASM::asmcode::CMP, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 0);
  code.add(ASM::asmcode::JLS, shift_right);
  code.add(ASM::asmcode::SHR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RCX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SAL, ASM::asmcode::RAX, ASM::asmcode::CL);
  code.add(ASM::asmcode::JMPS, done);
  code.add(ASM::asmcode::LABEL, shift_right);
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::NEG, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SHR, ASM::asmcode::RBX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RCX, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SAR, ASM::asmcode::RAX, ASM::asmcode::CL);
  code.add(ASM::asmcode::SAL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::LABEL, done);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RCX, ASM::asmcode::R15);
  }

void inline_quotient(ASM::asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(ASM::asmcode::TEST, ASM::asmcode::RBX, ASM::asmcode::RBX);
    code.add(ASM::asmcode::JES, div_by_zero);
    }
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::RDX);
  code.add(ASM::asmcode::XOR, ASM::asmcode::RDX, ASM::asmcode::RDX);
  code.add(ASM::asmcode::CQO);
  code.add(ASM::asmcode::IDIV, ASM::asmcode::RBX);
  code.add(ASM::asmcode::SHL, ASM::asmcode::RAX, ASM::asmcode::NUMBER, 1);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RDX, ASM::asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(ASM::asmcode::LABEL, done);
    }
  }

void inline_remainder(ASM::asmcode& code, const compiler_options& ops)
  {
  std::string done, div_by_zero;
  if (ops.safe_primitives)
    {
    div_by_zero = label_to_string(label++);
    done = label_to_string(label++);
    code.add(ASM::asmcode::TEST, ASM::asmcode::RBX, ASM::asmcode::RBX);
    code.add(ASM::asmcode::JES, div_by_zero);
    }
  code.add(ASM::asmcode::MOV, ASM::asmcode::R15, ASM::asmcode::RDX);
  code.add(ASM::asmcode::XOR, ASM::asmcode::RDX, ASM::asmcode::RDX);
  code.add(ASM::asmcode::CQO);
  code.add(ASM::asmcode::IDIV, ASM::asmcode::RBX);
  code.add(ASM::asmcode::MOV, ASM::asmcode::RAX, ASM::asmcode::RDX);
  code.add(ASM::asmcode::AND, ASM::asmcode::RAX, ASM::asmcode::NUMBER, ~((uint64_t)1));
  code.add(ASM::asmcode::MOV, ASM::asmcode::RDX, ASM::asmcode::R15);
  if (ops.safe_primitives)
    {
    code.add(ASM::asmcode::JMPS, done);
    error_label(code, div_by_zero, re_division_by_zero);
    code.add(ASM::asmcode::LABEL, done);
    }
  }

SKIWI_END
