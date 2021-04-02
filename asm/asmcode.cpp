#include "asmcode.h"
#include <sstream>
#include <cassert>
#include <array>
#include <vector>
#include <map>
#include <bitset>
#include <algorithm>

ASM_BEGIN

namespace
  {
  std::string uchar_to_hex(unsigned char i)
    {
    std::string hex;
    int h1 = (i >> 4) & 0x0f;
    if (h1 < 10)
      hex += '0' + char(h1);
    else
      hex += 'A' + char(h1) - 10;
    int h2 = (i) & 0x0f;
    if (h2 < 10)
      hex += '0' + char(h2);
    else
      hex += 'A' + char(h2) - 10;
    return hex;
    }

  std::string uint64_to_hex(uint64_t i)
    {
    if (i == 0)
      return std::string("0x00");
    unsigned char* ptr = (unsigned char*)&i;
    std::string out = "0x";
    bool skip = true;
    for (int j = 7; j >= 0; --j)
      {
      if (ptr[j])
        skip = false;
      if (!skip)
        out += uchar_to_hex(ptr[j]);
      }
    return out;
    }

  }

asmcode::asmcode()
  {
  instructions_list.emplace_back();
  instructions_list_stack.push_back(instructions_list.begin());
  }

asmcode::~asmcode()
  {
  }

void asmcode::push()
  {
  instructions_list.emplace_back();
  instructions_list_stack.push_back(--instructions_list.end());
  }

void asmcode::pop()
  {
  instructions_list_stack.pop_back();
  }

void asmcode::clear()
  {
  instructions_list_stack.clear();
  instructions_list.clear();
  instructions_list.emplace_back();
  instructions_list_stack.push_back(instructions_list.begin());
  }

const std::list<std::vector<asmcode::instruction>>& asmcode::get_instructions_list() const
  {
  return instructions_list;
  }

std::list<std::vector<asmcode::instruction>>& asmcode::get_instructions_list()
  {
  return instructions_list;
  }

void asmcode::stream(std::ostream& out) const
  {
  for (auto rit = instructions_list.rbegin(); rit != instructions_list.rend(); ++rit)
    {
    for (const auto& ins : *rit)
      {
      ins.stream(out);
      }
    }
  }

asmcode::instruction::instruction() : oper(NOP), operand1(EMPTY), operand2(EMPTY), operand1_mem(0), operand2_mem(0)
  {
  }

asmcode::instruction::instruction(const std::string& txt) : oper(COMMENT), operand1(EMPTY), operand2(EMPTY), operand1_mem(0), operand2_mem(0), text(txt)
  {
  }

asmcode::instruction::instruction(operation op) : oper(op), operand1(EMPTY), operand2(EMPTY), operand1_mem(0), operand2_mem(0)
  {
  }

asmcode::instruction::instruction(operation op, operand op1) : oper(op), operand1(op1), operand2(EMPTY), operand1_mem(0), operand2_mem(0)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, operand op2) : oper(op), operand1(op1), operand2(op2), operand1_mem(0), operand2_mem(0)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem) : oper(op), operand1(op1), operand2(EMPTY), operand1_mem(op1_mem), operand2_mem(0)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem, operand op2) : oper(op), operand1(op1), operand2(op2), operand1_mem(op1_mem), operand2_mem(0)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem) : oper(op), operand1(op1), operand2(op2), operand1_mem(op1_mem), operand2_mem(op2_mem)
  {
  }

asmcode::instruction::instruction(operation op, operand op1, operand op2, uint64_t op2_mem) : oper(op), operand1(op1), operand2(op2), operand1_mem(0), operand2_mem(op2_mem)
  {
  }

asmcode::instruction::instruction(operation op, const std::string& txt) : oper(op), operand1(EMPTY), operand2(EMPTY), operand1_mem(0), operand2_mem(0), text(txt)
  {
  }

asmcode::instruction::instruction(operation op, operand op1, const std::string& txt) : oper(op), operand1(op1), operand2(EMPTY), operand1_mem(0), operand2_mem(0), text(txt)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, operand op2, const std::string& txt) : oper(op), operand1(op1), operand2(op2), operand1_mem(0), operand2_mem(0), text(txt)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem, const std::string& txt) : oper(op), operand1(op1), operand2(EMPTY), operand1_mem(op1_mem), operand2_mem(0), text(txt)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem, operand op2, const std::string& txt) : oper(op), operand1(op1), operand2(op2), operand1_mem(op1_mem), operand2_mem(0), text(txt)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem, const std::string& txt) : oper(op), operand1(op1), operand2(op2), operand1_mem(op1_mem), operand2_mem(op2_mem), text(txt)
  {

  }

asmcode::instruction::instruction(operation op, operand op1, operand op2, uint64_t op2_mem, const std::string& txt) : oper(op), operand1(op1), operand2(op2), operand1_mem(0), operand2_mem(op2_mem), text(txt)
  {

  }

namespace
  {
  std::string _to_string(uint64_t mem)
    {
    std::stringstream str;
    str << mem;
    return str.str();
    }

  std::string _to_string_signed(int64_t mem)
    {
    std::stringstream str;
    str << mem;
    return str.str();
    }

  std::string _operation2string(asmcode::operation op)
    {
    switch (op)
      {
      case asmcode::ADD: return "add";
      case asmcode::ADDSD: return "addsd";
      case asmcode::AND: return "and";
      case asmcode::CMP: return "cmp";
      case asmcode::CMPEQPD: return "cmpeqpd";
      case asmcode::CMPLTPD: return "cmpltpd";
      case asmcode::CMPLEPD: return "cmplepd";
      case asmcode::CQO: return "cqo";
      case asmcode::CVTSI2SD: return "cvtsi2sd";
      case asmcode::CVTTSD2SI: return "cvttsd2si";
      case asmcode::DEC: return "dec";
      case asmcode::DIV: return "div";
      case asmcode::DIVSD: return "divsd";
      case asmcode::F2XM1: return "f2xm1";
      case asmcode::FADD: return "fadd";
      case asmcode::FILD: return "fild";
      case asmcode::FLD: return "fld";
      case asmcode::FLD1: return "fld1";
      case asmcode::FLDPI: return "fldpi";
      case asmcode::FLDLN2: return "fldln2";
      case asmcode::FMUL: return "fmul";
      case asmcode::FSIN: return "fsin";
      case asmcode::FCOS: return "fcos";
      case asmcode::FISTPQ: return "fistp qword";
      case asmcode::FPATAN: return "fpatan";
      case asmcode::FPTAN: return "fptan";
      case asmcode::FRNDINT: return "frndint";
      case asmcode::FSCALE: return "fscale";
      case asmcode::FSQRT: return "fsqrt";
      case asmcode::FSTP: return "fstp";
      case asmcode::FSUB: return "fsub";
      case asmcode::FSUBRP: return "fsubrp";
      case asmcode::FXCH: return "fxch";
      case asmcode::FYL2X: return "fyl2x";
      case asmcode::IDIV: return "idiv";
      case asmcode::IMUL: return "imul";
      case asmcode::INC: return "inc";
      case asmcode::MOV: return "mov";
      case asmcode::MOVQ: return "movq";
      case asmcode::MOVMSKPD: return "movmskpd";
      case asmcode::MOVSD: return "movsd";
      case asmcode::MOVZX: return "movzx";
      case asmcode::MUL: return "mul";
      case asmcode::MULSD: return "mulsd";
      case asmcode::NEG: return "neg";
      case asmcode::NOP: return "nop";
      case asmcode::OR: return "or";
      case asmcode::POP: return "pop";
      case asmcode::PUSH: return "push";
      case asmcode::RET: return "ret";
      case asmcode::SAL: return "sal";
      case asmcode::SAR: return "sar";
      case asmcode::SHL: return "shl";
      case asmcode::SETE: return "sete";
      case asmcode::SETNE: return "setne";
      case asmcode::SETL: return "setl";
      case asmcode::SETG: return "setg";
      case asmcode::SETLE: return "setle";
      case asmcode::SETGE: return "setge";
      case asmcode::SHR: return "shr";
      case asmcode::SQRTPD: return "sqrtpd";
      case asmcode::SUB: return "sub";
      case asmcode::SUBSD: return "subsd";
      case asmcode::TEST: return "test";
      case asmcode::UCOMISD: return "ucomisd";
      case asmcode::XOR: return "xor";
      case asmcode::XORPD: return "xorpd";
      }
    return "";
    }

  std::string _operand2string(asmcode::operand op, uint64_t mem, const std::string& text)
    {
    switch (op)
      {
      case asmcode::AL: return "al";
      case asmcode::AH: return "ah";
      case asmcode::BL: return "bl";
      case asmcode::BH: return "bh";
      case asmcode::CL: return "cl";
      case asmcode::CH: return "ch";
      case asmcode::DL: return "dl";
      case asmcode::DH: return "dh";
      case asmcode::RAX: return "rax";
      case asmcode::RBX: return "rbx";
      case asmcode::RCX: return "rcx";
      case asmcode::RDX: return "rdx";
      case asmcode::RDI: return "rdi";
      case asmcode::RSI: return "rsi";
      case asmcode::RSP: return "rsp";
      case asmcode::RBP: return "rbp";
      case asmcode::R8: return "r8";
      case asmcode::R9: return "r9";
      case asmcode::R10: return "r10";
      case asmcode::R11: return "r11";
      case asmcode::R12: return "r12";
      case asmcode::R13: return "r13";
      case asmcode::R14: return "r14";
      case asmcode::R15: return "r15";
      case asmcode::ST0: return "st0";
      case asmcode::ST1: return "st1";
      case asmcode::ST2: return "st2";
      case asmcode::ST3: return "st3";
      case asmcode::ST4: return "st4";
      case asmcode::ST5: return "st5";
      case asmcode::ST6: return "st6";
      case asmcode::ST7: return "st7";
      case asmcode::XMM0: return "xmm0";
      case asmcode::XMM1: return "xmm1";
      case asmcode::XMM2: return "xmm2";
      case asmcode::XMM3: return "xmm3";
      case asmcode::XMM4: return "xmm4";
      case asmcode::XMM5: return "xmm5";
      case asmcode::XMM6: return "xmm6";
      case asmcode::XMM7: return "xmm7";
      case asmcode::XMM8: return "xmm8";
      case asmcode::XMM9: return "xmm9";
      case asmcode::XMM10: return "xmm10";
      case asmcode::XMM11: return "xmm11";
      case asmcode::XMM12: return "xmm12";
      case asmcode::XMM13: return "xmm13";
      case asmcode::XMM14: return "xmm14";
      case asmcode::XMM15: return "xmm15";
      case asmcode::NUMBER: return uint64_to_hex(mem);
      case asmcode::MEM_RAX: return mem ? ("[rax" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rax]";
      case asmcode::MEM_RBX: return mem ? ("[rbx" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rbx]";
      case asmcode::MEM_RCX: return mem ? ("[rcx" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rcx]";
      case asmcode::MEM_RDX: return mem ? ("[rdx" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rdx]";
      case asmcode::MEM_RDI: return mem ? ("[rdi" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rdi]";
      case asmcode::MEM_RSI: return mem ? ("[rsi" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rsi]";
      case asmcode::MEM_RSP: return mem ? ("[rsp" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rsp]";
      case asmcode::MEM_RBP: return mem ? ("[rbp" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[rbp]";
      case asmcode::MEM_R8: return  mem ? ("[r8" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r8]";
      case asmcode::MEM_R9: return  mem ? ("[r9" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r9]";
      case asmcode::MEM_R10: return mem ? ("[r10" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r10]";
      case asmcode::MEM_R11: return mem ? ("[r11" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r11]";
      case asmcode::MEM_R12: return mem ? ("[r12" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r13]";
      case asmcode::MEM_R13: return mem ? ("[r13" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r13]";
      case asmcode::MEM_R14: return mem ? ("[r14" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r14]";
      case asmcode::MEM_R15: return mem ? ("[r15" + ((int64_t(mem)) > 0 ? ("+" + _to_string(mem)) : _to_string_signed(mem)) + "]") : "[r15]";
      case asmcode::BYTE_MEM_RAX: return mem ? ("byte [rax" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rax]";
      case asmcode::BYTE_MEM_RBX: return mem ? ("byte [rbx" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rbx]";
      case asmcode::BYTE_MEM_RCX: return mem ? ("byte [rcx" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rcx]";
      case asmcode::BYTE_MEM_RDX: return mem ? ("byte [rdx" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rdx]";
      case asmcode::BYTE_MEM_RDI: return mem ? ("byte [rdi" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rdi]";
      case asmcode::BYTE_MEM_RSI: return mem ? ("byte [rsi" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rsi]";
      case asmcode::BYTE_MEM_RSP: return mem ? ("byte [rsp" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rsp]";
      case asmcode::BYTE_MEM_RBP: return mem ? ("byte [rbp" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [rbp]";
      case asmcode::BYTE_MEM_R8: return  mem ? ("byte [r8" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r8]";
      case asmcode::BYTE_MEM_R9: return  mem ? ("byte [r9" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r9]";
      case asmcode::BYTE_MEM_R10: return mem ? ("byte [r10" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r10]";
      case asmcode::BYTE_MEM_R11: return mem ? ("byte [r11" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r11]";
      case asmcode::BYTE_MEM_R12: return mem ? ("byte [r12" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r12]";
      case asmcode::BYTE_MEM_R13: return mem ? ("byte [r13" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r13]";
      case asmcode::BYTE_MEM_R14: return mem ? ("byte [r14" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r14]";
      case asmcode::BYTE_MEM_R15: return mem ? ("byte [r15" + ((int64_t(mem)) > 0 ? (" + " + _to_string(mem)) : _to_string_signed(mem)) + "]") : "byte [r15]";
      case asmcode::LABELADDRESS: return text;
      }
    return "";
    }
  }

void asmcode::instruction::stream(std::ostream& out) const
  {
  switch (oper)
    {
    case COMMENT:
    {
    out << "; " << text << std::endl;
    break;
    }
    case GLOBAL:
    {
    out << "global " << text << std::endl;
    out << "SECTION .text" << std::endl;
    out << text << ":" << std::endl;
    break;
    }
    case LABEL:
    {
    out << text << ":" << std::endl;
    break;
    }
    case LABEL_ALIGNED:
    {
    out << text << ":" << std::endl;
    break;
    }
    case EXTERN:
    {
    out << "extern " << text << std::endl;
    break;
    }
    case CALLEXTERNAL:
    case CALL:
    {
    out << "\tcall";
    if (operand1 != asmcode::EMPTY)
      {
      out << " " << _operand2string(operand1, operand1_mem, text);
      }
    else
      out << " " << text;
    out << std::endl;
    break;
    }
    case JMP:
    {
    out << "\tjmp";
    if (operand1 != asmcode::EMPTY)
      {
      out << " " << _operand2string(operand1, operand1_mem, text);
      }
    else
      out << " " << text;
    out << std::endl;
    break;
    }
    case JE:
    {
    out << "\tje " << text << std::endl;
    break;
    }
    case JNE:
    {
    out << "\tjne " << text << std::endl;
    break;
    }
    case JL:
    {
    out << "\tjl " << text << std::endl;
    break;
    }
    case JLE:
    {
    out << "\tjle " << text << std::endl;
    break;
    }
    case JA:
    {
    out << "\tja " << text << std::endl;
    break;
    }
    case JB:
    {
    out << "\tjb " << text << std::endl;
    break;
    }
    case JG:
    {
    out << "\tjg " << text << std::endl;
    break;
    }
    case JGE:
    {
    out << "\tjge " << text << std::endl;
    break;
    }
    case JMPS:
    {
    out << "\tjmp short " << text << std::endl;
    break;
    }
    case JES:
    {
    out << "\tje short " << text << std::endl;
    break;
    }
    case JNES:
    {
    out << "\tjne short " << text << std::endl;
    break;
    }
    case JLS:
    {
    out << "\tjl short " << text << std::endl;
    break;
    }
    case JLES:
    {
    out << "\tjle short " << text << std::endl;
    break;
    }
    case JAS:
    {
    out << "\tja short " << text << std::endl;
    break;
    }
    case JBS:
    {
    out << "\tjb short " << text << std::endl;
    break;
    }
    case JGS:
    {
    out << "\tjg short " << text << std::endl;
    break;
    }
    case JGES:
    {
    out << "\tjge short " << text << std::endl;
    break;
    }
    default:
    {
    out << "\t" << _operation2string(oper);
    if (operand1 != asmcode::EMPTY)
      {
      out << " " << _operand2string(operand1, operand1_mem, text);
      }
    if (operand2 != asmcode::EMPTY)
      {
      out << ", " << _operand2string(operand2, operand2_mem, text);
      }
    out << std::endl;
    break;
    }
    }
  }

namespace
  {
  uint8_t get_rex(uint8_t w, uint8_t r, uint8_t x, uint8_t b)
    {
    uint8_t rex = 64;
    if (w)
      rex |= 8;
    if (r)
      rex |= 4;
    if (x)
      rex |= 2;
    if (b)
      rex |= 1;
    return rex;
    }

  void add_rex_b(uint8_t& rex)
    {
    rex |= 1;
    }

  void add_rex_x(uint8_t& rex)
    {
    rex |= 2;
    }

  void add_rex_r(uint8_t& rex)
    {
    rex |= 4;
    }

  uint8_t make_modrm_byte(uint8_t mod, uint8_t reg, uint8_t rm)
    {
    assert(mod < 4);
    assert(reg < 8);
    assert(rm < 8);
    return (mod << 6) | (reg << 3) | rm;
    }

  uint8_t get_mod(uint8_t modrm)
    {
    return (modrm >> 6);
    }

  uint8_t get_rm(uint8_t modrm)
    {
    return (modrm & 7);
    }

  uint8_t make_sib_byte(uint8_t scale, uint8_t index, uint8_t base)
    {
    assert(scale < 4);
    assert(index < 8);
    assert(base < 8);
    return (scale << 6) | (index << 3) | base;
    }

  bool is_8_bit(uint64_t number)
    {
    int8_t b = int8_t(number);
    return int64_t(b) == (int64_t)number;
    }

  bool is_16_bit(uint64_t number)
    {
    int16_t b = int16_t(number);
    return int64_t(b) == (int64_t)number;
    }

  bool is_32_bit(uint64_t number)
    {
    int32_t b = int32_t(number);
    return int64_t(b) == (int64_t)number;
    }

  uint8_t get_modrm_byte_with_digit(const asmcode::operand& target, uint8_t& rex, uint8_t digit, uint64_t target_offset)
    {
    uint8_t mod = 0;
    uint8_t reg = digit;
    uint8_t rm = 0;
    bool eight_bit = is_8_bit(target_offset);
    bool zero = target_offset == 0;
    switch (target)
      {
      case asmcode::XMM0:
      case asmcode::AL:
      case asmcode::RAX: mod = 3; rm = 0;  break;
      case asmcode::XMM1:
      case asmcode::CL:
      case asmcode::RCX: mod = 3; rm = 1;  break;
      case asmcode::XMM2:
      case asmcode::DL:
      case asmcode::RDX: mod = 3; rm = 2;  break;
      case asmcode::XMM3:
      case asmcode::BL:
      case asmcode::RBX: mod = 3; rm = 3;  break;
      case asmcode::XMM4:
      case asmcode::AH:
      case asmcode::RSP: mod = 3; rm = 4;  break;
      case asmcode::XMM5:
      case asmcode::CH:
      case asmcode::RBP: mod = 3; rm = 5;  break;
      case asmcode::XMM6:
      case asmcode::DH:
      case asmcode::RSI: mod = 3; rm = 6;  break;
      case asmcode::XMM7:
      case asmcode::BH:
      case asmcode::RDI: mod = 3; rm = 7;  break;
      case asmcode::XMM8:
      case asmcode::R8: mod = 3; rm = 0; add_rex_b(rex); break;
      case asmcode::XMM9:
      case asmcode::R9: mod = 3; rm = 1; add_rex_b(rex); break;
      case asmcode::XMM10:
      case asmcode::R10: mod = 3; rm = 2; add_rex_b(rex); break;
      case asmcode::XMM11:
      case asmcode::R11: mod = 3; rm = 3; add_rex_b(rex); break;
      case asmcode::XMM12:
      case asmcode::R12: mod = 3; rm = 4; add_rex_b(rex); break;
      case asmcode::XMM13:
      case asmcode::R13: mod = 3; rm = 5; add_rex_b(rex); break;
      case asmcode::XMM14:
      case asmcode::R14: mod = 3; rm = 6; add_rex_b(rex); break;
      case asmcode::XMM15:
      case asmcode::R15: mod = 3; rm = 7; add_rex_b(rex); break;
      case asmcode::MEM_RAX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; break;
      case asmcode::MEM_RCX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; break;
      case asmcode::MEM_RDX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; break;
      case asmcode::MEM_RBX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; break;
      case asmcode::MEM_RSP: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; break;
      case asmcode::MEM_RBP: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; break; // rbp is exception for zero offset
      case asmcode::MEM_RSI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; break;
      case asmcode::MEM_RDI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; break;
      case asmcode::MEM_R8: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; add_rex_b(rex); break;
      case asmcode::MEM_R9: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; add_rex_b(rex); break;
      case asmcode::MEM_R10: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; add_rex_b(rex); break;
      case asmcode::MEM_R11: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; add_rex_b(rex); break;
      case asmcode::MEM_R12: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; add_rex_b(rex); break;
      case asmcode::MEM_R13: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 5; add_rex_b(rex); break;
      case asmcode::MEM_R14: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; add_rex_b(rex); break;
      case asmcode::MEM_R15: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; add_rex_b(rex); break;

        // not tested very well, bugs possible
      case asmcode::BYTE_MEM_RAX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; break;
      case asmcode::BYTE_MEM_RCX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; break;
      case asmcode::BYTE_MEM_RDX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; break;
      case asmcode::BYTE_MEM_RBX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; break;
      case asmcode::BYTE_MEM_RSP: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; break;
      case asmcode::BYTE_MEM_RBP: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; break; // rbp is exception for zero offset
      case asmcode::BYTE_MEM_RSI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; break;
      case asmcode::BYTE_MEM_RDI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; break;
      case asmcode::BYTE_MEM_R8: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R9: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R10: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R11: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R12: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R13: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 5; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R14: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R15: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; add_rex_b(rex); break;
      default: throw std::logic_error("get_modrm_byte: not implemented");
      }
    return make_modrm_byte(mod, reg, rm);
    }

  uint8_t get_modrm_byte(const asmcode::operand& target, const asmcode::operand& source, uint8_t& rex, uint64_t target_offset)
    {
    uint8_t mod = 0;
    uint8_t reg = 0;
    uint8_t rm = 0;
    bool eight_bit = is_8_bit(target_offset);
    bool zero = target_offset == 0;
    switch (target)
      {
      case asmcode::XMM0:
      case asmcode::AL:
      case asmcode::RAX: mod = 3; rm = 0;  break;
      case asmcode::XMM1:
      case asmcode::CL:
      case asmcode::RCX: mod = 3; rm = 1;  break;
      case asmcode::XMM2:
      case asmcode::DL:
      case asmcode::RDX: mod = 3; rm = 2;  break;
      case asmcode::XMM3:
      case asmcode::BL:
      case asmcode::RBX: mod = 3; rm = 3;  break;
      case asmcode::XMM4:
      case asmcode::AH:
      case asmcode::RSP: mod = 3; rm = 4;  break;
      case asmcode::XMM5:
      case asmcode::CH:
      case asmcode::RBP: mod = 3; rm = 5;  break;
      case asmcode::XMM6:
      case asmcode::DH:
      case asmcode::RSI: mod = 3; rm = 6;  break;
      case asmcode::XMM7:
      case asmcode::BH:
      case asmcode::RDI: mod = 3; rm = 7;  break;
      case asmcode::XMM8:
      case asmcode::R8: mod = 3; rm = 0; add_rex_b(rex); break;
      case asmcode::XMM9:
      case asmcode::R9: mod = 3; rm = 1; add_rex_b(rex); break;
      case asmcode::XMM10:
      case asmcode::R10: mod = 3; rm = 2; add_rex_b(rex); break;
      case asmcode::XMM11:
      case asmcode::R11: mod = 3; rm = 3; add_rex_b(rex); break;
      case asmcode::XMM12:
      case asmcode::R12: mod = 3; rm = 4; add_rex_b(rex); break;
      case asmcode::XMM13:
      case asmcode::R13: mod = 3; rm = 5; add_rex_b(rex); break;
      case asmcode::XMM14:
      case asmcode::R14: mod = 3; rm = 6; add_rex_b(rex); break;
      case asmcode::XMM15:
      case asmcode::R15: mod = 3; rm = 7; add_rex_b(rex); break;
      case asmcode::MEM_RAX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; break;
      case asmcode::MEM_RCX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; break;
      case asmcode::MEM_RDX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; break;
      case asmcode::MEM_RBX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; break;
      case asmcode::MEM_RSP: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; break;
      case asmcode::MEM_RBP: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; break; // rbp is exception for zero offset
      case asmcode::MEM_RSI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; break;
      case asmcode::MEM_RDI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; break;
      case asmcode::MEM_R8: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; add_rex_b(rex); break;
      case asmcode::MEM_R9: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; add_rex_b(rex); break;
      case asmcode::MEM_R10: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; add_rex_b(rex); break;
      case asmcode::MEM_R11: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; add_rex_b(rex); break;
      case asmcode::MEM_R12: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; add_rex_b(rex); break;
      case asmcode::MEM_R13: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; add_rex_b(rex); break; // r13 is exception for zero offset
      case asmcode::MEM_R14: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; add_rex_b(rex); break;
      case asmcode::MEM_R15: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; add_rex_b(rex); break;

        // not tested very well, bugs possible
      case asmcode::BYTE_MEM_RAX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; break;
      case asmcode::BYTE_MEM_RCX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; break;
      case asmcode::BYTE_MEM_RDX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; break;
      case asmcode::BYTE_MEM_RBX: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; break;
      case asmcode::BYTE_MEM_RSP: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; break;
      case asmcode::BYTE_MEM_RBP: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; break; // rbp is exception for zero offset
      case asmcode::BYTE_MEM_RSI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; break;
      case asmcode::BYTE_MEM_RDI: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; break;
      case asmcode::BYTE_MEM_R8: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 0; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R9: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 1; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R10: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 2; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R11: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 3; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R12: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 4; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R13: mod = zero ? 1 : (eight_bit ? 1 : 2); rm = 5; add_rex_b(rex); break; // r13 is exception for zero offset
      case asmcode::BYTE_MEM_R14: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 6; add_rex_b(rex); break;
      case asmcode::BYTE_MEM_R15: mod = zero ? 0 : (eight_bit ? 1 : 2); rm = 7; add_rex_b(rex); break;
      default: throw std::logic_error("get_modrm_byte: not implemented");
      }
    switch (source)
      {
      case asmcode::AL:
      case asmcode::XMM0:
      case asmcode::RAX: reg = 0;  break;
      case asmcode::XMM1:
      case asmcode::CL:
      case asmcode::RCX: reg = 1;  break;
      case asmcode::XMM2:
      case asmcode::DL:
      case asmcode::RDX: reg = 2;  break;
      case asmcode::XMM3:
      case asmcode::BL:
      case asmcode::RBX: reg = 3;  break;
      case asmcode::XMM4:
      case asmcode::AH:
      case asmcode::RSP: reg = 4;  break;
      case asmcode::XMM5:
      case asmcode::CH:
      case asmcode::RBP: reg = 5;  break;
      case asmcode::XMM6:
      case asmcode::DH:
      case asmcode::RSI: reg = 6;  break;
      case asmcode::XMM7:
      case asmcode::BH:
      case asmcode::RDI: reg = 7;  break;
      case asmcode::XMM8:
      case asmcode::R8: add_rex_r(rex); reg = 0; break;
      case asmcode::XMM9:
      case asmcode::R9: add_rex_r(rex); reg = 1; break;
      case asmcode::XMM10:
      case asmcode::R10: add_rex_r(rex); reg = 2; break;
      case asmcode::XMM11:
      case asmcode::R11: add_rex_r(rex); reg = 3; break;
      case asmcode::XMM12:
      case asmcode::R12: add_rex_r(rex); reg = 4; break;
      case asmcode::XMM13:
      case asmcode::R13: add_rex_r(rex); reg = 5; break;
      case asmcode::XMM14:
      case asmcode::R14: add_rex_r(rex); reg = 6; break;
      case asmcode::XMM15:
      case asmcode::R15: add_rex_r(rex); reg = 7; break;
      default: throw std::logic_error("get_modrm_byte: not implemented");
      }
    return make_modrm_byte(mod, reg, rm);
    }

  void add_register_to_opcode(uint8_t& rex, uint8_t& opcode, const asmcode::operand& op)
    {
    switch (op)
      {
      case asmcode::AL: break;
      case asmcode::RAX: break;
      case asmcode::CL:
      case asmcode::RCX: opcode |= 1; break;
      case asmcode::BL:
      case asmcode::RDX: opcode |= 2; break;
      case asmcode::DL:
      case asmcode::RBX: opcode |= 3; break;
      case asmcode::AH:
      case asmcode::RSP: opcode |= 4; break;
      case asmcode::CH:
      case asmcode::RBP: opcode |= 5; break;
      case asmcode::DH:
      case asmcode::RSI: opcode |= 6; break;
      case asmcode::BH:
      case asmcode::RDI: opcode |= 7; break;
      case asmcode::R8: add_rex_b(rex); break;
      case asmcode::R9: add_rex_b(rex); opcode |= 1; break;
      case asmcode::R10: add_rex_b(rex); opcode |= 2; break;
      case asmcode::R11: add_rex_b(rex); opcode |= 3; break;
      case asmcode::R12: add_rex_b(rex); opcode |= 4; break;
      case asmcode::R13: add_rex_b(rex); opcode |= 5; break;
      case asmcode::R14: add_rex_b(rex); opcode |= 6; break;
      case asmcode::R15: add_rex_b(rex); opcode |= 7; break;
      default: throw std::logic_error("add_register_to_opcode: this register is not implemented");
      }
    }

  void push1byte(uint8_t*& opcode_stream, uint8_t value)
    {
    *opcode_stream = value;
    ++opcode_stream;
    }

  void push2byte(uint8_t*& opcode_stream, uint16_t value)
    {
    *((uint16_t*)opcode_stream) = value;
    opcode_stream += 2;
    }

  void push4byte(uint8_t*& opcode_stream, uint32_t value)
    {
    *((uint32_t*)opcode_stream) = value;
    opcode_stream += 4;
    }

  void push8byte(uint8_t*& opcode_stream, uint64_t value)
    {
    *((uint64_t*)opcode_stream) = value;
    opcode_stream += 8;
    }


  struct opcode
    {
    enum opcode_operand_type
      {
      none = 0,
      m8 = 0x1,
      m16 = 0x2,
      m32 = 0x4,
      m64 = 0x8,
      r8 = 0x10,
      r16 = 0x20,
      r32 = 0x40,
      r64 = 0x80,
      rm8 = 0x11,
      rm16 = 0x22,
      rm32 = 0x44,
      rm64 = 0x88,
      imm8 = 0xF00,
      imm16 = 0xE00,
      imm32 = 0xC00,
      imm64 = 0x800,
      m128 = 0x1000,
      xmm = 0x2000,
      xmm_m64 = 0x2008,
      xmm_m128 = 0x3008,
      sti = 0x4000,
      st0 = 0xC000,
      rax = 0x10080,
      al = 0x100010,
      cl = 0x200010,
      };

    enum opcode_flags
      {
      rexw = 1,
      r = 2,
      digit0 = 4,
      digit1 = 8,
      digit2 = 16,
      digit3 = 32,
      digit4 = 64,
      digit5 = 128,
      digit6 = 256,
      digit7 = 512,
      ib = 1024,
      iw = 2048,
      id = 4096,
      io = 8192,
      rb = 16384,
      rw = 32768,
      rd = 65536,
      ro = 0x20000,
      cb = 0x40000,
      cw = 0x80000,
      cd = 0x100000,
      co = 0x200000
      };


    uint8_t prefix, postfix;
    uint8_t opcode_id;
    uint8_t opcode_id_2;
    uint64_t flags;

    bool use_postfix;

    opcode_operand_type operand_1, operand_2, operand_3;
    std::string mnemonic;   
    };


  opcode make_opcode(uint8_t prefix, std::string mnemonic, uint64_t flags, uint8_t opcode_id, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2)
    {
    opcode o;
    o.prefix = prefix;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = 0;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = opcode::none;
    o.use_postfix = false;
    return o;
    }


  opcode make_opcode(uint8_t prefix, std::string mnemonic, uint64_t flags, uint8_t opcode_id, uint8_t opcode_id_2, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2)
    {
    opcode o;
    o.prefix = prefix;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = opcode_id_2;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = opcode::none;
    o.use_postfix = false;
    return o;
    }

  opcode make_opcode(uint8_t prefix, std::string mnemonic, uint64_t flags, uint8_t opcode_id, uint8_t opcode_id_2, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2, uint8_t postfix)
    {
    opcode o;
    o.prefix = prefix;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = opcode_id_2;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = opcode::none;
    o.use_postfix = true;
    o.postfix = postfix;
    return o;
    }

  opcode make_opcode(std::string mnemonic, uint64_t flags, uint8_t opcode_id, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2)
    {
    opcode o;
    o.prefix = 0;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = 0;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = opcode::none;
    o.use_postfix = false;
    return o;
    }

  opcode make_opcode(std::string mnemonic, uint64_t flags, uint8_t opcode_id, uint8_t opcode_id_2, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2, opcode::opcode_operand_type op3)
    {
    opcode o;
    o.prefix = 0;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = opcode_id_2;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = op3;
    o.use_postfix = false;
    return o;
    }

  opcode make_opcode(std::string mnemonic, uint64_t flags, uint8_t opcode_id, uint8_t opcode_id_2, opcode::opcode_operand_type op1, opcode::opcode_operand_type op2)
    {
    opcode o;
    o.prefix = 0;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = opcode_id_2;
    o.operand_1 = op1;
    o.operand_2 = op2;
    o.operand_3 = opcode::none;
    o.use_postfix = false;
    return o;
    }

  opcode make_opcode(std::string mnemonic, uint64_t flags, uint8_t opcode_id, uint8_t opcode_id_2, opcode::opcode_operand_type op1)
    {
    opcode o;
    o.prefix = 0;
    o.mnemonic = mnemonic;
    o.flags = flags;
    o.opcode_id = opcode_id;
    o.opcode_id_2 = opcode_id_2;
    o.operand_1 = op1;
    o.operand_2 = opcode::none;
    o.operand_3 = opcode::none;
    o.use_postfix = false;
    return o;
    }


  uint64_t hamming_distance(uint64_t left, uint64_t right)
    {
    return std::bitset<64>(left ^ right).count();
    }

  struct opcode_table
    {

    std::vector<opcode> opcodes;

    void add_opcode(opcode op)
      {
      opcodes.push_back(op);
      }

    opcode find_opcode(opcode::opcode_operand_type op1, opcode::opcode_operand_type op2) const
      {
      //auto it = std::find_if(opcodes.begin(), opcodes.end(), [&](const opcode& op) { return ((op.operand_1 & op1) == op.operand_1) && ((op.operand_2 & op2) == op.operand_2); });

      std::vector<opcode> matches;
      for (const auto& op : opcodes)
        {
        //if (((op.operand_1 & op1) == op.operand_1) && ((op.operand_2 & op2) == op.operand_2))
        //  matches.push_back(op);
        if (((op.operand_1 & op1) || (op.operand_1 == op1)) && ((op.operand_2 & op2) || (op.operand_2 == op2)))
          matches.push_back(op);
        }

      if (matches.empty())
        throw std::logic_error("opcode_table: not available");

      if (matches.size() == 1)
        return matches.front();

      /*
      For now this sorting works.
      Alternatively: Fill buffer for all matches, and choose shortest one.
      */
      std::sort(matches.begin(), matches.end(), [&](const opcode& op_left, const opcode& op_right)
        {
        uint64_t ham_left_1 = hamming_distance(op_left.operand_1, op1);
        uint64_t ham_left_2 = hamming_distance(op_left.operand_2, op2);
        uint64_t ham_right_1 = hamming_distance(op_right.operand_1, op1);
        uint64_t ham_right_2 = hamming_distance(op_right.operand_2, op2);
        uint64_t val_left = ham_left_1 + ham_left_2 * 64;
        uint64_t val_right = ham_right_1 + ham_right_2 * 64;
        if (val_left == val_right)
          {
          val_left = op_left.operand_1 + op_left.operand_2 * 0x100000;
          val_right = op_right.operand_1 + op_right.operand_2 * 0x100000;
          }
        return val_left < val_right;
        });

      return matches.front();

      }
    };

  bool is_memory_operand_type(opcode::opcode_operand_type op)
    {
    return (op == opcode::m8 || op == opcode::m16 || op == opcode::m32 || op == opcode::m64 || op == opcode::m128);
    }

  bool is_rm_operand_type(opcode::opcode_operand_type op)
    {
    return (op == opcode::rm8 || op == opcode::rm16 || op == opcode::rm32 || op == opcode::rm64 || op == opcode::xmm_m64 || op == opcode::xmm_m128);
    }

  bool is_immediate_operand_type(opcode::opcode_operand_type op)
    {
    return (op == opcode::imm8 || op == opcode::imm16 || op == opcode::imm32 || op == opcode::imm64);
    }

  opcode_table make_mov_table()
    {
    opcode_table t;

    t.add_opcode(make_opcode("MOV", opcode::rexw | opcode::r, 0x89, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("MOV", opcode::rexw | opcode::r, 0x8B, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("MOV", opcode::rexw | opcode::rd | opcode::io, 0xB8, opcode::r64, opcode::imm64));
    t.add_opcode(make_opcode("MOV", opcode::rexw | opcode::id | opcode::digit0, 0xC7, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("MOV", opcode::id | opcode::digit0, 0xC7, opcode::rm32, opcode::imm32));

    t.add_opcode(make_opcode("MOV", opcode::r, 0x8A, opcode::r8, opcode::rm8));

    t.add_opcode(make_opcode("MOV", opcode::r, 0x88, opcode::rm8, opcode::r8));

    t.add_opcode(make_opcode("MOV", opcode::digit0 | opcode::ib, 0xC6, opcode::rm8, opcode::imm8));

    t.add_opcode(make_opcode("MOV", opcode::rd | opcode::id, 0xB8, opcode::r32, opcode::imm32));
    t.add_opcode(make_opcode("MOV", opcode::r, 0x89, opcode::rm32, opcode::r32));
    t.add_opcode(make_opcode("MOV", opcode::r, 0x8B, opcode::r32, opcode::rm32));

    return t;
    }

  opcode_table make_movq_table()
    {
    opcode_table t;

    t.add_opcode(make_opcode(0x66, "MOVQ", opcode::rexw | opcode::r, 0x0f, 0x7e, opcode::rm64, opcode::xmm));
    //t.add_opcode(make_opcode(0x66, "MOVQ", opcode::rexw | opcode::r, 0x0f, 0x7e, opcode::m64, opcode::xmm));

    t.add_opcode(make_opcode(0x66, "MOVQ", opcode::rexw | opcode::r, 0x0f, 0x6e, opcode::xmm, opcode::rm64));
    //t.add_opcode(make_opcode(0x66, "MOVQ", opcode::rexw | opcode::r, 0x0f, 0x6e, opcode::xmm, opcode::m64));
    return t;
    }

  
  opcode_table make_add_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("ADD", opcode::rexw | opcode::id, 0x05, opcode::rax, opcode::imm32));

    t.add_opcode(make_opcode("ADD", opcode::rexw | opcode::digit0 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("ADD", opcode::digit0 | opcode::id, 0x81, opcode::rm32, opcode::imm32));

    t.add_opcode(make_opcode("ADD", opcode::rexw | opcode::digit0 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("ADD", opcode::digit0 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));


    t.add_opcode(make_opcode("ADD", opcode::rexw | opcode::r, 0x01, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("ADD", opcode::r, 0x01, opcode::rm32, opcode::r32));

    t.add_opcode(make_opcode("ADD", opcode::rexw | opcode::r, 0x03, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("ADD", opcode::r, 0x03, opcode::r32, opcode::rm32));

    t.add_opcode(make_opcode("ADD", opcode::ib, 0x04, opcode::al, opcode::imm8));
    t.add_opcode(make_opcode("ADD", opcode::ib | opcode::digit0, 0x80, opcode::rm8, opcode::imm8));
    return t;
    }

  opcode_table make_sub_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SUB", opcode::rexw | opcode::id, 0x2D, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("SUB", opcode::rexw | opcode::digit5 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("SUB", opcode::digit5 | opcode::id, 0x81, opcode::rm32, opcode::imm32));

    t.add_opcode(make_opcode("SUB", opcode::rexw | opcode::digit5 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("SUB", opcode::digit5 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));

    t.add_opcode(make_opcode("SUB", opcode::rexw | opcode::r, 0x29, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("SUB", opcode::r, 0x29, opcode::rm32, opcode::r32));

    t.add_opcode(make_opcode("SUB", opcode::rexw | opcode::r, 0x2B, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("SUB", opcode::r, 0x2B, opcode::r32, opcode::rm32));

    t.add_opcode(make_opcode("SUB", opcode::ib, 0x2C, opcode::al, opcode::imm8));
    t.add_opcode(make_opcode("SUB", opcode::ib | opcode::digit5, 0x80, opcode::rm8, opcode::imm8));
    return t;
    }

  opcode_table make_and_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("AND", opcode::rexw | opcode::id, 0x25, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("AND", opcode::rexw | opcode::digit4 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("AND", opcode::rexw | opcode::digit4 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("AND", opcode::digit4 | opcode::id, 0x81, opcode::rm32, opcode::imm32));
    t.add_opcode(make_opcode("AND", opcode::digit4 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));
    t.add_opcode(make_opcode("AND", opcode::rexw | opcode::r, 0x21, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("AND", opcode::rexw | opcode::r, 0x23, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("AND", opcode::r, 0x21, opcode::rm32, opcode::r32));
    t.add_opcode(make_opcode("AND", opcode::r, 0x23, opcode::r32, opcode::rm32));
    t.add_opcode(make_opcode("AND", opcode::digit4 | opcode::ib, 0x80, opcode::rm8, opcode::imm8));
    t.add_opcode(make_opcode("AND", opcode::ib, 0x24, opcode::al, opcode::imm8));
    return t;
    }

  opcode_table make_or_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("OR", opcode::rexw | opcode::id, 0x0D, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("OR", opcode::rexw | opcode::digit1 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("OR", opcode::rexw | opcode::digit1 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("OR", opcode::digit1 | opcode::id, 0x81, opcode::rm32, opcode::imm32));
    t.add_opcode(make_opcode("OR", opcode::digit1 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));
    t.add_opcode(make_opcode("OR", opcode::rexw | opcode::r, 0x09, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("OR", opcode::rexw | opcode::r, 0x0B, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("OR", opcode::r, 0x09, opcode::rm32, opcode::r32));
    t.add_opcode(make_opcode("OR", opcode::r, 0x0B, opcode::r32, opcode::rm32));

    t.add_opcode(make_opcode("OR", opcode::r, 0x08, opcode::rm8, opcode::r8));
    return t;
    }

  opcode_table make_xor_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("XOR", opcode::rexw | opcode::id, 0x35, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("XOR", opcode::rexw | opcode::digit6 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("XOR", opcode::rexw | opcode::digit6 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("XOR", opcode::digit6 | opcode::id, 0x81, opcode::rm32, opcode::imm32));
    t.add_opcode(make_opcode("XOR", opcode::digit6 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));

    t.add_opcode(make_opcode("XOR", opcode::rexw | opcode::r, 0x31, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("XOR", opcode::r, 0x31, opcode::rm32, opcode::r32));

    t.add_opcode(make_opcode("XOR", opcode::rexw | opcode::r, 0x33, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("XOR", opcode::r, 0x33, opcode::r32, opcode::rm32));

    t.add_opcode(make_opcode("XOR", opcode::digit6 | opcode::ib, 0x80, opcode::rm8, opcode::imm8));
    return t;
    }

  opcode_table make_xorpd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("XORPD", opcode::r, 0x0f, 0x57, opcode::xmm, opcode::xmm_m128));
    return t;
    }

  opcode_table make_cmp_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("CMP", opcode::rexw | opcode::id, 0x3D, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("CMP", opcode::rexw | opcode::digit7 | opcode::id, 0x81, opcode::rm64, opcode::imm32));
    t.add_opcode(make_opcode("CMP", opcode::rexw | opcode::digit7 | opcode::ib, 0x83, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("CMP", opcode::digit7 | opcode::id, 0x81, opcode::rm32, opcode::imm32));
    t.add_opcode(make_opcode("CMP", opcode::digit7 | opcode::ib, 0x83, opcode::rm32, opcode::imm8));
    t.add_opcode(make_opcode("CMP", opcode::rexw | opcode::r, 0x39, opcode::rm64, opcode::r64));
    t.add_opcode(make_opcode("CMP", opcode::r, 0x39, opcode::rm32, opcode::r32));
    t.add_opcode(make_opcode("CMP", opcode::rexw | opcode::r, 0x3B, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("CMP", opcode::r, 0x3B, opcode::r32, opcode::rm32));

    t.add_opcode(make_opcode("CMP", opcode::r, 0x38, opcode::rm8, opcode::r8));
    t.add_opcode(make_opcode("CMP", opcode::digit7 | opcode::ib, 0x80, opcode::rm8, opcode::imm8));
    t.add_opcode(make_opcode("CMP", opcode::ib, 0x3c, opcode::al, opcode::imm8));
    return t;
    }

  opcode_table make_test_table()
    {
    opcode_table t;

    t.add_opcode(make_opcode("TEST", opcode::rexw | opcode::id, 0xA9, opcode::rax, opcode::imm32));
    t.add_opcode(make_opcode("TEST", opcode::rexw | opcode::digit0 | opcode::id, 0xF7, opcode::rm64, opcode::imm32));    
    t.add_opcode(make_opcode("TEST", opcode::digit0 | opcode::id, 0xF7, opcode::rm32, opcode::imm32));

    t.add_opcode(make_opcode("TEST", opcode::rexw | opcode::r, 0x85, opcode::rm64, opcode::r64));

    t.add_opcode(make_opcode("TEST", opcode::r, 0x84, opcode::rm8, opcode::r8));

    //85 /r TEST r/m32, r32
    t.add_opcode(make_opcode("TEST", opcode::r, 0x85, opcode::rm32, opcode::r32));
    return t;
    }

  opcode_table make_ucomisd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "UCOMISD", opcode::r, 0x0f, 0x2E, opcode::xmm, opcode::xmm_m64));
    return t;
    }

  opcode_table make_jmp_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JMP", opcode::cd, 0xe9, opcode::imm32, opcode::none));

    t.add_opcode(make_opcode("JMP", opcode::digit4, 0xFF, opcode::rm64, opcode::none));
    return t;
    }

  opcode_table make_je_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JE", opcode::cd, 0x0f, 0x84, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jl_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JL", opcode::cd, 0x0f, 0x8C, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jle_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JLE", opcode::cd, 0x0f, 0x8E, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jg_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JG", opcode::cd, 0x0f, 0x8F, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jb_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JB", opcode::cd, 0x0f, 0x82, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_ja_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JA", opcode::cd, 0x0f, 0x87, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jge_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JGE", opcode::cd, 0x0f, 0x8D, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jne_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JNE", opcode::cd, 0x0f, 0x85, opcode::imm32, opcode::none));
    return t;
    }

  opcode_table make_jmps_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JMPS", opcode::cb, 0xeb, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jes_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JES", opcode::cb, 0x74, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jls_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JLS", opcode::cb, 0x7C, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jles_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JLES", opcode::cb, 0x7E, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jbs_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JBS", opcode::cd, 0x72, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jas_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JAS", opcode::cd, 0x77, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jgs_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JGS", opcode::cb, 0x7F, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_jges_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JGES", opcode::cb, 0x7D, opcode::imm8, opcode::none));
    return t;
    }


  opcode_table make_jnes_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("JNES", opcode::cb, 0x75, opcode::imm8, opcode::none));
    return t;
    }

  opcode_table make_sete_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETE", opcode::r, 0x0f, 0x94, opcode::rm8));

    return t;
    }

  opcode_table make_setne_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETNE", opcode::r, 0x0f, 0x95, opcode::rm8));

    return t;
    }

  opcode_table make_setl_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETL", opcode::r, 0x0f, 0x9c, opcode::rm8));

    return t;
    }

  opcode_table make_setg_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETG", opcode::r, 0x0f, 0x9f, opcode::rm8));

    return t;
    }

  opcode_table make_setle_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETLE", opcode::r, 0x0f, 0x9e, opcode::rm8));

    return t;
    }

  opcode_table make_setge_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SETGE", opcode::r, 0x0f, 0x9d, opcode::rm8));

    return t;
    }

  opcode_table make_shl_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SHL", opcode::rexw | opcode::digit4 | opcode::ib, 0xC1, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("SHL", opcode::digit4 | opcode::ib, 0xC1, opcode::rm32, opcode::imm8));
    t.add_opcode(make_opcode("SHL", opcode::rexw | opcode::digit4, 0xD3, opcode::rm64, opcode::cl));
    return t;
    }

  opcode_table make_shr_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SHR", opcode::rexw | opcode::digit5 | opcode::ib, 0xC1, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("SHR", opcode::digit5 | opcode::ib, 0xC1, opcode::rm32, opcode::imm8));
    t.add_opcode(make_opcode("SHR", opcode::rexw | opcode::digit5, 0xD3, opcode::rm64, opcode::cl));
    return t;
    }

  opcode_table make_sal_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SAL", opcode::rexw | opcode::digit4 | opcode::ib, 0xC1, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("SAL", opcode::rexw | opcode::digit4, 0xD3, opcode::rm64, opcode::cl));
    return t;
    }

  opcode_table make_sar_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("SAR", opcode::rexw | opcode::digit7 | opcode::ib, 0xC1, opcode::rm64, opcode::imm8));
    t.add_opcode(make_opcode("SAR", opcode::rexw | opcode::digit7, 0xD3, opcode::rm64, opcode::cl));
    return t;
    }

  opcode_table make_call_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("CALL", opcode::digit2, 0xff, opcode::rm64, opcode::none));
    //t.add_opcode(make_opcode("CALL", opcode::digit2, 0xff, opcode::m64, opcode::none));
    t.add_opcode(make_opcode("CALL", opcode::cd, 0xe8, opcode::imm32, opcode::none));
    //t.add_opcode(make_opcode("CALL", opcode::cd, 0xe8, opcode::imm64, opcode::none));
    return t;
    }

  opcode_table make_push_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("PUSH", opcode::rd, 0x50, opcode::rm64, opcode::none));
    //t.add_opcode(make_opcode("PUSH", opcode::digit6, 0xff, opcode::m64, opcode::none));
    return t;
    }


  opcode_table make_pop_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("POP", opcode::rd, 0x58, opcode::rm64, opcode::none));
    //t.add_opcode(make_opcode("POP", opcode::digit0, 0x8f, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_ret_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("RET", 0, 0xc3, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_cqo_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("CQO", opcode::rexw, 0x99, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_dec_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("DEC", opcode::rexw | opcode::digit1, 0xff, opcode::rm64, opcode::none));
    t.add_opcode(make_opcode("DEC", opcode::digit1, 0xff, opcode::rm32, opcode::none));
    return t;
    }

  opcode_table make_inc_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("INC", opcode::rexw | opcode::digit0, 0xff, opcode::rm64, opcode::none));
    t.add_opcode(make_opcode("INC", opcode::digit0, 0xff, opcode::rm32, opcode::none));
    return t;
    }

  opcode_table make_idiv_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("IDIV", opcode::rexw | opcode::digit7, 0xf7, opcode::rm64, opcode::none));
    t.add_opcode(make_opcode("IDIV", opcode::digit7, 0xf7, opcode::rm32, opcode::none));
    return t;
    }

  opcode_table make_div_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("DIV", opcode::rexw | opcode::digit6, 0xf7, opcode::rm64, opcode::none));
    //t.add_opcode(make_opcode("DIV", opcode::rexw | opcode::digit6, 0xf7, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_mul_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("MUL", opcode::rexw | opcode::digit4, 0xf7, opcode::rm64, opcode::none));
    //t.add_opcode(make_opcode("MUL", opcode::rexw | opcode::digit4, 0xf7, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_imul_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("IMUL", opcode::rexw | opcode::digit5, 0xf7, opcode::rm64, opcode::none));
    t.add_opcode(make_opcode("IMUL", opcode::rexw | opcode::r, 0x0f, 0xaf, opcode::r64, opcode::rm64));
    t.add_opcode(make_opcode("IMUL", opcode::r, 0x0f, 0xaf, opcode::r32, opcode::rm32));
    return t;
    }

  opcode_table make_nop_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("NOP", 0, 0x90, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_neg_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("NEG", opcode::rexw | opcode::digit3, 0xf7, opcode::rm64, opcode::none));
    t.add_opcode(make_opcode("NEG", opcode::digit3, 0xf7, opcode::rm32, opcode::none));
    return t;
    }

  opcode_table make_cvtsi2sd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "CVTSI2SD", opcode::rexw | opcode::r, 0x0f, 0x2a, opcode::xmm, opcode::rm64));
    return t;
    }
  
  opcode_table make_cvttsd2si_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "CVTTSD2SI", opcode::rexw | opcode::r, 0x0f, 0x2c, opcode::r64, opcode::xmm_m64));
    return t;
    }
  
  opcode_table make_addsd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "ADDSD", opcode::r, 0x0f, 0x58, opcode::xmm, opcode::xmm_m64));
    //t.add_opcode(make_opcode(0xf2, "ADDSD", opcode::r, 0x0f, 0x58, opcode::xmm, opcode::m64));
    return t;
    }

  opcode_table make_divsd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "DIVSD", opcode::r, 0x0f, 0x5E, opcode::xmm, opcode::xmm_m64));
    //t.add_opcode(make_opcode(0xf2, "DIVSD", opcode::r, 0x0f, 0x5E, opcode::xmm, opcode::m64));
    return t;
    }

  opcode_table make_mulsd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "MULSD", opcode::r, 0x0f, 0x59, opcode::xmm, opcode::xmm_m64));
    //t.add_opcode(make_opcode(0xf2, "MULSD", opcode::r, 0x0f, 0x59, opcode::xmm, opcode::m64));
    return t;
    }

  
  opcode_table make_subsd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "SUBSD", opcode::r, 0x0f, 0x5C, opcode::xmm, opcode::xmm_m64));
    //t.add_opcode(make_opcode(0xf2, "SUBSD", opcode::r, 0x0f, 0x5C, opcode::xmm, opcode::m64));
    return t;
    }
  
  opcode_table make_cmpeqpd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "CMPEQPD", opcode::r | opcode::ib, 0x0f, 0xC2, opcode::xmm, opcode::xmm_m128, 0x0));
    return t;
    }

  opcode_table make_cmpltpd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "CMPLTPD", opcode::r | opcode::ib, 0x0f, 0xC2, opcode::xmm, opcode::xmm_m128, 0x1));
    return t;
    }

  opcode_table make_cmplepd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "CMPLEPD", opcode::r | opcode::ib, 0x0f, 0xC2, opcode::xmm, opcode::xmm_m128, 0x2));
    return t;
    }

  opcode_table make_movmskpd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "MOVMSKPD", opcode::r, 0x0f, 0x50, opcode::r64, opcode::xmm_m128));
    t.add_opcode(make_opcode(0x66, "MOVMSKPD", opcode::r, 0x0f, 0x50, opcode::r32, opcode::xmm_m128));
    return t;
    }

 
  opcode_table make_movsd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0xf2, "MOVSD", opcode::r, 0x0f, 0x10, opcode::xmm, opcode::xmm_m64));
    t.add_opcode(make_opcode(0xf2, "MOVSD", opcode::r, 0x0f, 0x11, opcode::xmm_m64, opcode::xmm));
    return t;
    }

  opcode_table make_movzx_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("MOVZX", opcode::rexw | opcode::r, 0x0f, 0xB6, opcode::r64, opcode::rm8));
    t.add_opcode(make_opcode("MOVZX", opcode::r, 0x0f, 0xB6, opcode::r32, opcode::rm8));
    return t;
    }

  opcode_table make_sqrtpd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode(0x66, "SQRTPD", opcode::r, 0x0f, 0x51, opcode::xmm, opcode::xmm_m128));
    return t;
    }

  opcode_table make_f2xm1_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("F2XM1", 0, 0xd9, 0xf0, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fadd_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FADD", opcode::digit0, 0xD8, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FADD", opcode::digit0, 0xDC, opcode::m64, opcode::none));
    t.add_opcode(make_opcode("FADD", 0, 0xD8, 0xC0, opcode::st0, opcode::sti));
    t.add_opcode(make_opcode("FADD", 0, 0xDC, 0xC0, opcode::sti, opcode::st0));
    t.add_opcode(make_opcode("FADD", 0, 0xDE, 0xC1, opcode::none, opcode::none));
    return t;
    }

  
  opcode_table make_fistp_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FISTP", opcode::digit3, 0xDB, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FISTP", opcode::digit7, 0xDF, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_fld_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FLD", opcode::digit0, 0xd9, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FLD", opcode::digit0, 0xdd, opcode::m64, opcode::none));
    t.add_opcode(make_opcode("FLD", 0, 0xd9, 0xc0, opcode::sti, opcode::none));
    return t;
    }

  opcode_table make_fild_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FILD", opcode::digit0, 0xdf, opcode::m16, opcode::none));
    t.add_opcode(make_opcode("FILD", opcode::digit0, 0xdb, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FILD", opcode::digit5, 0xdf, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_fld1_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FLD1", 0, 0xd9, 0xe8, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fldpi_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FLDPI", 0, 0xd9, 0xeb, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fldln2_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FLDLN2", 0, 0xd9, 0xed, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fmul_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FMUL", 0, 0xDC, 0xC8, opcode::sti, opcode::st0));
    t.add_opcode(make_opcode("FMUL", opcode::digit1, 0xD8, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FMUL", opcode::digit1, 0xDC, opcode::m64, opcode::none));

    return t;
    }
  
  opcode_table make_fsubrp_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSUBRP", 0, 0xde, 0xe1, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fsqrt_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSQRT", 0, 0xd9, 0xfa, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fsin_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSIN", 0, 0xd9, 0xfe, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fcos_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSIN", 0, 0xd9, 0xff, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fpatan_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FPATAN", 0, 0xd9, 0xf3, opcode::none, opcode::none));
    return t;
    }
  
  opcode_table make_fptan_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FPTAN", 0, 0xd9, 0xf2, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_frndint_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FRNDINT", 0, 0xd9, 0xfc, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fscale_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSCALE", 0, 0xd9, 0xfd, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fstp_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSTP", 0, 0xdd, 0xd8, opcode::sti, opcode::none));
    t.add_opcode(make_opcode("FSTP", opcode::digit3, 0xd9, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FSTP", opcode::digit3, 0xdd, opcode::m64, opcode::none));
    return t;
    }

  opcode_table make_fsub_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FSUB", opcode::digit4, 0xD8, opcode::m32, opcode::none));
    t.add_opcode(make_opcode("FSUB", opcode::digit4, 0xDC, opcode::m64, opcode::none));
    t.add_opcode(make_opcode("FSUB", 0, 0xD8, 0xE0, opcode::st0, opcode::sti));
    t.add_opcode(make_opcode("FSUB", 0, 0xDC, 0xE8, opcode::sti, opcode::st0));
    t.add_opcode(make_opcode("FSUB", 0, 0xDE, 0xE9, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fxch_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FXCH", 0, 0xd9, 0xc8, opcode::sti, opcode::none));
    t.add_opcode(make_opcode("FXCH", 0, 0xd9, 0xc9, opcode::none, opcode::none));
    return t;
    }

  opcode_table make_fyl2x_table()
    {
    opcode_table t;
    t.add_opcode(make_opcode("FYL2X", 0, 0xd9, 0xf1, opcode::none, opcode::none));
    return t;
    }


  std::map<std::string, opcode_table> make_opcode_table()
    {
    std::map<std::string, opcode_table> table;
    table["ADD"] = make_add_table();
    table["ADDSD"] = make_addsd_table();    
    table["AND"] = make_and_table();
    table["CALL"] = make_call_table();   
    table["CMP"] = make_cmp_table();
    table["CMPEQPD"] = make_cmpeqpd_table();
    table["CMPLTPD"] = make_cmpltpd_table();
    table["CMPLEPD"] = make_cmplepd_table();
    table["CQO"] = make_cqo_table();        
    table["CVTSI2SD"] = make_cvtsi2sd_table();    
    table["CVTTSD2SI"] = make_cvttsd2si_table();    
    table["DEC"] = make_dec_table();
    table["DIV"] = make_div_table();
    table["DIVSD"] = make_divsd_table();    

    table["F2XM1"] = make_f2xm1_table();    
    table["FADD"] = make_fadd_table();   
    table["FISTP"] = make_fistp_table();    
    table["FLD"] = make_fld_table();
    table["FILD"] = make_fild_table();
    table["FLD1"] = make_fld1_table();  
    table["FLDPI"] = make_fldpi_table();
    table["FLDLN2"] = make_fldln2_table();    
    table["FMUL"] = make_fmul_table();
    table["FSIN"] = make_fsin_table();
    table["FCOS"] = make_fcos_table();
    table["FPATAN"] = make_fpatan_table();    
    table["FPTAN"] = make_fptan_table();
    table["FRNDINT"] = make_frndint_table();
    table["FSCALE"] = make_fscale_table();
    table["FSQRT"] = make_fsqrt_table();
    table["FSTP"] = make_fstp_table();
    table["FSUB"] = make_fsub_table();    
    table["FSUBRP"] = make_fsubrp_table();
    table["FXCH"] = make_fxch_table();
    table["FYL2X"] = make_fyl2x_table();

    table["IDIV"] = make_idiv_table();
    table["IMUL"] = make_imul_table();
    table["INC"] = make_inc_table();
    table["JA"] = make_ja_table();
    table["JB"] = make_jb_table();
    table["JE"] = make_je_table();
    table["JL"] = make_jl_table();
    table["JLE"] = make_jle_table();
    table["JG"] = make_jg_table();
    table["JGE"] = make_jge_table();
    table["JNE"] = make_jne_table();
    table["JMP"] = make_jmp_table();
    table["JES"] = make_jes_table();
    table["JLS"] = make_jls_table();
    table["JLES"] = make_jles_table();
    table["JAS"] = make_jas_table();
    table["JBS"] = make_jbs_table();
    table["JGS"] = make_jgs_table();
    table["JGES"] = make_jges_table();
    table["JNES"] = make_jnes_table();
    table["JMPS"] = make_jmps_table();
    table["MOV"] = make_mov_table();        
    table["MOVQ"] = make_movq_table();    
    table["MOVMSKPD"] = make_movmskpd_table();    
    table["MOVSD"] = make_movsd_table();
    table["MOVZX"] = make_movzx_table();
    table["MUL"] = make_mul_table();
    table["MULSD"] = make_mulsd_table();    
    table["NEG"] = make_neg_table();
    table["NOP"] = make_nop_table();
    table["OR"] = make_or_table();
    table["POP"] = make_pop_table();
    table["PUSH"] = make_push_table();
    table["RET"] = make_ret_table();
    table["SAL"] = make_sal_table();
    table["SAR"] = make_sar_table();
    table["SETE"] = make_sete_table();
    table["SETNE"] = make_setne_table();
    table["SETL"] = make_setl_table();
    table["SETG"] = make_setg_table();
    table["SETLE"] = make_setle_table();
    table["SETGE"] = make_setge_table();
    table["SHL"] = make_shl_table();    
    table["SHR"] = make_shr_table();
    table["SQRTPD"] = make_sqrtpd_table();    
    table["SUB"] = make_sub_table();
    table["SUBSD"] = make_subsd_table();    
    table["TEST"] = make_test_table();
    table["UCOMISD"] = make_ucomisd_table();   
    table["XOR"] = make_xor_table();
    table["XORPD"] = make_xorpd_table();    
    return table;
    }

  opcode::opcode_operand_type get_opcode_operand_type(const asmcode::operand& op, uint64_t nr)
    {
    switch (op)
      {
      case asmcode::EMPTY: return opcode::none;
      case asmcode::AL: return opcode::al;
      case asmcode::AH: return opcode::r8;
      case asmcode::BL: return opcode::r8;
      case asmcode::BH: return opcode::r8;
      case asmcode::CL: return opcode::cl;
      case asmcode::CH: return opcode::r8;
      case asmcode::DL: return opcode::r8;
      case asmcode::DH: return opcode::r8;      
      case asmcode::RAX: return opcode::rax;
      case asmcode::RBX: return opcode::r64;
      case asmcode::RCX: return opcode::r64;
      case asmcode::RDX: return opcode::r64;
      case asmcode::RDI: return opcode::r64;
      case asmcode::RSI: return opcode::r64;
      case asmcode::RSP: return opcode::r64;
      case asmcode::RBP: return opcode::r64;
      case asmcode::R8:  return opcode::r64;
      case asmcode::R9:  return opcode::r64;
      case asmcode::R10: return opcode::r64;
      case asmcode::R11: return opcode::r64;
      case asmcode::R12: return opcode::r64;
      case asmcode::R13: return opcode::r64;
      case asmcode::R14: return opcode::r64;
      case asmcode::R15: return opcode::r64;     
      case asmcode::MEM_RAX: return opcode::m64;
      case asmcode::MEM_RBX: return opcode::m64;
      case asmcode::MEM_RCX: return opcode::m64;
      case asmcode::MEM_RDX: return opcode::m64;
      case asmcode::MEM_RDI: return opcode::m64;
      case asmcode::MEM_RSI: return opcode::m64;
      case asmcode::MEM_RSP: return opcode::m64;
      case asmcode::MEM_RBP: return opcode::m64;
      case asmcode::MEM_R8:  return opcode::m64;
      case asmcode::MEM_R9:  return opcode::m64;
      case asmcode::MEM_R10: return opcode::m64;
      case asmcode::MEM_R11: return opcode::m64;
      case asmcode::MEM_R12: return opcode::m64;
      case asmcode::MEM_R13: return opcode::m64;
      case asmcode::MEM_R14: return opcode::m64;
      case asmcode::MEM_R15: return opcode::m64;
      case asmcode::BYTE_MEM_RAX: return opcode::m8;
      case asmcode::BYTE_MEM_RBX: return opcode::m8;
      case asmcode::BYTE_MEM_RCX: return opcode::m8;
      case asmcode::BYTE_MEM_RDX: return opcode::m8;
      case asmcode::BYTE_MEM_RDI: return opcode::m8;
      case asmcode::BYTE_MEM_RSI: return opcode::m8;
      case asmcode::BYTE_MEM_RSP: return opcode::m8;
      case asmcode::BYTE_MEM_RBP: return opcode::m8;
      case asmcode::BYTE_MEM_R8:  return opcode::m8;
      case asmcode::BYTE_MEM_R9:  return opcode::m8;
      case asmcode::BYTE_MEM_R10: return opcode::m8;
      case asmcode::BYTE_MEM_R11: return opcode::m8;
      case asmcode::BYTE_MEM_R12: return opcode::m8;
      case asmcode::BYTE_MEM_R13: return opcode::m8;
      case asmcode::BYTE_MEM_R14: return opcode::m8;
      case asmcode::BYTE_MEM_R15: return opcode::m8;    
      case asmcode::NUMBER:
      {
      if (is_8_bit(nr))
        return opcode::imm8;
      else if (is_16_bit(nr))
        return opcode::imm16;
      else if (is_32_bit(nr))
        return opcode::imm32;
      else
        return opcode::imm64;
      }
      case asmcode::ST0:  return opcode::st0;
      case asmcode::ST1:  return opcode::sti;
      case asmcode::ST2:  return opcode::sti;
      case asmcode::ST3:  return opcode::sti;
      case asmcode::ST4:  return opcode::sti;
      case asmcode::ST5:  return opcode::sti;
      case asmcode::ST6:  return opcode::sti;
      case asmcode::ST7:  return opcode::sti;
      case asmcode::XMM0: return opcode::xmm;
      case asmcode::XMM1: return opcode::xmm;
      case asmcode::XMM2: return opcode::xmm;
      case asmcode::XMM3: return opcode::xmm;
      case asmcode::XMM4: return opcode::xmm;
      case asmcode::XMM5: return opcode::xmm;
      case asmcode::XMM6: return opcode::xmm;
      case asmcode::XMM7: return opcode::xmm;
      case asmcode::XMM8: return opcode::xmm;
      case asmcode::XMM9: return opcode::xmm;
      case asmcode::XMM10: return opcode::xmm;
      case asmcode::XMM11: return opcode::xmm;
      case asmcode::XMM12: return opcode::xmm;
      case asmcode::XMM13: return opcode::xmm;
      case asmcode::XMM14: return opcode::xmm;
      case asmcode::XMM15: return opcode::xmm; 
      case asmcode::LABELADDRESS: return opcode::imm64;
      }
    return opcode::none;
    }

  uint64_t fill_default(uint8_t* opcode_stream, asmcode::instruction code, opcode o, opcode::opcode_operand_type op1d, opcode::opcode_operand_type op2d)
    {    

    uint8_t* stream = opcode_stream;

    bool use_modrm = false;

    uint8_t rex = 0;
    uint8_t modrm = 0;
    uint8_t sib = 0;

    if (o.flags & opcode::rexw)
      rex = get_rex(1, 0, 0, 0);

    if (o.prefix)
      push1byte(stream, o.prefix);

    uint8_t c = o.opcode_id;

    if (o.flags & (opcode::rb | opcode::rw | opcode::rd | opcode::ro))
      add_register_to_opcode(rex, c, code.operand1);

    if (o.flags & opcode::r)
      {
      if (is_rm_operand_type(o.operand_2))
        modrm = get_modrm_byte(code.operand2, code.operand1, rex, code.operand2_mem);
      else if (code.operand2 != asmcode::operand::EMPTY)
        modrm = get_modrm_byte(code.operand1, code.operand2, rex, code.operand1_mem);
      else
        modrm = get_modrm_byte(code.operand1, code.operand1, rex, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit0)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 0, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit1)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 1, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit2)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 2, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit3)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 3, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit4)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 4, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit5)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 5, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit6)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 6, code.operand1_mem);
      use_modrm = true;
      }
    else if (o.flags & opcode::digit7)
      {
      modrm = get_modrm_byte_with_digit(code.operand1, rex, 7, code.operand1_mem);
      use_modrm = true;
      }

    if (use_modrm)
      {
      //if (code.operand1 == asmcode::MEM_RSP || code.operand2 == asmcode::MEM_RSP || code.operand1 == asmcode::BYTE_MEM_RSP || code.operand2 == asmcode::BYTE_MEM_RSP) // sib necessary
      if ((get_rm(modrm) == 4) && (get_mod(modrm) != 3))
        {
        sib = make_sib_byte(0, 4, 4);
        }
      }

    if (rex)
      {
      rex |= 64; // first 4 bits should be 0100
      push1byte(stream, rex);
      }

    push1byte(stream, c);
    if (o.opcode_id_2)
      {
      if (o.operand_1 == opcode::sti)
        {
        uint8_t i = (uint8_t)(code.operand1 - asmcode::ST0);
        push1byte(stream, o.opcode_id_2 + i);
        }
      else if (o.operand_2 == opcode::sti)
        {
        uint8_t i = (uint8_t)(code.operand2 - asmcode::ST0);
        push1byte(stream, o.opcode_id_2 + i);
        }
      else
        push1byte(stream, o.opcode_id_2);
      }
    if (use_modrm)
      push1byte(stream, modrm);
    if (sib)
      push1byte(stream, sib);
    if (is_memory_operand_type(op1d))
      {
      if (code.operand1_mem || code.operand1 == asmcode::MEM_RBP || code.operand1 == asmcode::MEM_R13 || code.operand1 == asmcode::BYTE_MEM_RBP || code.operand1 == asmcode::BYTE_MEM_R13) // [rbp] and [r13] are exception 
        {
        if (is_8_bit(code.operand1_mem))
          push1byte(stream, (uint8_t)code.operand1_mem);
        else
          push4byte(stream, (uint32_t)code.operand1_mem);
        }
      }
    else if (is_immediate_operand_type(op1d))
      {
      switch (o.operand_1)
        {
        case opcode::rm8:
        case opcode::imm8: push1byte(stream, (uint8_t)code.operand1_mem); break;
        case opcode::rm16:
        case opcode::imm16: push2byte(stream, (uint16_t)code.operand1_mem); break;
        case opcode::rm32:
        case opcode::imm32: push4byte(stream, (uint32_t)code.operand1_mem); break;
        case opcode::rm64:
        case opcode::imm64: push8byte(stream, (uint64_t)code.operand1_mem); break;
        }
      }
    if (is_immediate_operand_type(op2d))
      {
      switch (o.operand_2)
        {
        case opcode::rm8:
        case opcode::imm8: push1byte(stream, (uint8_t)code.operand2_mem); break;
        case opcode::rm16:
        case opcode::imm16: push2byte(stream, (uint16_t)code.operand2_mem); break;
        case opcode::rm32:
        case opcode::imm32: push4byte(stream, (uint32_t)code.operand2_mem); break;
        case opcode::rm64:
        case opcode::imm64: push8byte(stream, (uint64_t)code.operand2_mem); break;
        }
      }
    else if (is_memory_operand_type(op2d))
      {
      if (code.operand2_mem || code.operand2 == asmcode::MEM_RBP || code.operand2 == asmcode::MEM_R13 || code.operand2 == asmcode::BYTE_MEM_RBP || code.operand2 == asmcode::BYTE_MEM_R13) // [rbp] and [r13] are exception 
        {
        if (is_8_bit(code.operand2_mem))
          push1byte(stream, (uint8_t)code.operand2_mem);
        else
          push4byte(stream, (uint32_t)code.operand2_mem);
        }
      }
    if (o.use_postfix)
      push1byte(stream, o.postfix);

    return stream - opcode_stream;
    }

  bool mem_is_64_bit_or_higher(opcode::opcode_operand_type op)
    {
    return ((op & opcode::m64) == opcode::m64);
    }

  uint64_t get_compressed_displacement(opcode::opcode_operand_type op)
    {
    /*
    Table 2-34 and 2-35 in Intel guide. Todo
    */
    if ((op & opcode::m32) == opcode::m32)
      return 4;
    return 64;
    }

  uint64_t fill(uint8_t* opcode_stream, const asmcode::instruction& code, const opcode_table& table)
    {
    auto op1d = get_opcode_operand_type(code.operand1, code.operand1_mem);
    auto op2d = get_opcode_operand_type(code.operand2, code.operand2_mem);
    auto o = table.find_opcode(op1d, op2d);

    return fill_default(opcode_stream, code, o, op1d, op2d);      
    }

  std::map<std::string, opcode_table> g_table = make_opcode_table();
  }

uint64_t asmcode::instruction::fill_opcode(uint8_t* opcode_stream) const
  {
  switch (oper)
    {
    case asmcode::ADD: return fill(opcode_stream, *this, g_table.find("ADD")->second);
    case asmcode::ADDSD: return fill(opcode_stream, *this, g_table.find("ADDSD")->second);
    case asmcode::AND: return fill(opcode_stream, *this, g_table.find("AND")->second);
    case asmcode::CALLEXTERNAL:
    case asmcode::CALL: return fill(opcode_stream, *this, g_table.find("CALL")->second);    
    case asmcode::COMMENT: return 0;
    case asmcode::CMP: return fill(opcode_stream, *this, g_table.find("CMP")->second);
    case asmcode::CMPEQPD: return fill(opcode_stream, *this, g_table.find("CMPEQPD")->second);
    case asmcode::CMPLTPD: return fill(opcode_stream, *this, g_table.find("CMPLTPD")->second);
    case asmcode::CMPLEPD: return fill(opcode_stream, *this, g_table.find("CMPLEPD")->second);
    case asmcode::CQO: return fill(opcode_stream, *this, g_table.find("CQO")->second);
    case asmcode::CVTSI2SD: return fill(opcode_stream, *this, g_table.find("CVTSI2SD")->second);    
    case asmcode::CVTTSD2SI: return fill(opcode_stream, *this, g_table.find("CVTTSD2SI")->second);
    case asmcode::DEC: return fill(opcode_stream, *this, g_table.find("DEC")->second);
    case asmcode::DIV: return fill(opcode_stream, *this, g_table.find("DIV")->second);
    case asmcode::DIVSD: return fill(opcode_stream, *this, g_table.find("DIVSD")->second);
    case asmcode::EXTERN: return 0;
    case asmcode::F2XM1: return fill(opcode_stream, *this, g_table.find("F2XM1")->second);
    case asmcode::FADD: return fill(opcode_stream, *this, g_table.find("FADD")->second);   
    case asmcode::FISTPQ: return fill(opcode_stream, *this, g_table.find("FISTP")->second);   
    case asmcode::FILD: return fill(opcode_stream, *this, g_table.find("FILD")->second);
    case asmcode::FLD: return fill(opcode_stream, *this, g_table.find("FLD")->second);
    case asmcode::FLD1: return fill(opcode_stream, *this, g_table.find("FLD1")->second);   
    case asmcode::FLDPI: return fill(opcode_stream, *this, g_table.find("FLDPI")->second);
    case asmcode::FLDLN2: return fill(opcode_stream, *this, g_table.find("FLDLN2")->second);   
    case asmcode::FMUL: return fill(opcode_stream, *this, g_table.find("FMUL")->second);
    case asmcode::FSIN: return fill(opcode_stream, *this, g_table.find("FSIN")->second);
    case asmcode::FCOS: return fill(opcode_stream, *this, g_table.find("FCOS")->second);
    case asmcode::FPATAN: return fill(opcode_stream, *this, g_table.find("FPATAN")->second);  
    case asmcode::FPTAN: return fill(opcode_stream, *this, g_table.find("FPTAN")->second);
    case asmcode::FRNDINT: return fill(opcode_stream, *this, g_table.find("FRNDINT")->second);
    case asmcode::FSCALE: return fill(opcode_stream, *this, g_table.find("FSCALE")->second);
    case asmcode::FSQRT: return fill(opcode_stream, *this, g_table.find("FSQRT")->second);
    case asmcode::FSTP: return fill(opcode_stream, *this, g_table.find("FSTP")->second);    
    case asmcode::FSUB: return fill(opcode_stream, *this, g_table.find("FSUB")->second);   
    case asmcode::FSUBRP: return fill(opcode_stream, *this, g_table.find("FSUBRP")->second);
    case asmcode::FXCH: return fill(opcode_stream, *this, g_table.find("FXCH")->second);
    case asmcode::FYL2X: return fill(opcode_stream, *this, g_table.find("FYL2X")->second);
    case asmcode::GLOBAL: return 0;
    case asmcode::LABEL: return 0;

    case asmcode::LABEL_ALIGNED: return 0;
    case asmcode::IDIV: return fill(opcode_stream, *this, g_table.find("IDIV")->second);
    case asmcode::IMUL: return fill(opcode_stream, *this, g_table.find("IMUL")->second);
    case asmcode::INC: return fill(opcode_stream, *this, g_table.find("INC")->second);
    case asmcode::JE: return fill(opcode_stream, *this, g_table.find("JE")->second);
    case asmcode::JL: return fill(opcode_stream, *this, g_table.find("JL")->second);
    case asmcode::JLE: return fill(opcode_stream, *this, g_table.find("JLE")->second);
    case asmcode::JA: return fill(opcode_stream, *this, g_table.find("JA")->second);
    case asmcode::JB: return fill(opcode_stream, *this, g_table.find("JB")->second);
    case asmcode::JG: return fill(opcode_stream, *this, g_table.find("JG")->second);
    case asmcode::JGE: return fill(opcode_stream, *this, g_table.find("JGE")->second);
    case asmcode::JNE: return fill(opcode_stream, *this, g_table.find("JNE")->second);
    case asmcode::JMP: return fill(opcode_stream, *this, g_table.find("JMP")->second);
    case asmcode::JES: return fill(opcode_stream, *this, g_table.find("JES")->second);
    case asmcode::JLS: return fill(opcode_stream, *this, g_table.find("JLS")->second);
    case asmcode::JLES: return fill(opcode_stream, *this, g_table.find("JLES")->second);
    case asmcode::JAS: return fill(opcode_stream, *this, g_table.find("JAS")->second);
    case asmcode::JBS: return fill(opcode_stream, *this, g_table.find("JBS")->second);
    case asmcode::JGS: return fill(opcode_stream, *this, g_table.find("JGS")->second);
    case asmcode::JGES: return fill(opcode_stream, *this, g_table.find("JGES")->second);
    case asmcode::JNES: return fill(opcode_stream, *this, g_table.find("JNES")->second);
    case asmcode::JMPS: return fill(opcode_stream, *this, g_table.find("JMPS")->second); 
    case asmcode::MOV: return fill(opcode_stream, *this, g_table.find("MOV")->second);   
    case asmcode::MOVQ: return fill(opcode_stream, *this, g_table.find("MOVQ")->second);   
    case asmcode::MOVMSKPD: return fill(opcode_stream, *this, g_table.find("MOVMSKPD")->second);   
    case asmcode::MOVSD: return fill(opcode_stream, *this, g_table.find("MOVSD")->second);
    case asmcode::MOVZX: return fill(opcode_stream, *this, g_table.find("MOVZX")->second);
    case asmcode::MUL: return fill(opcode_stream, *this, g_table.find("MUL")->second);
    case asmcode::MULSD: return fill(opcode_stream, *this, g_table.find("MULSD")->second);   
    case asmcode::NEG: return fill(opcode_stream, *this, g_table.find("NEG")->second);
    case asmcode::NOP: return fill(opcode_stream, *this, g_table.find("NOP")->second);
    case asmcode::OR: return fill(opcode_stream, *this, g_table.find("OR")->second);
    case asmcode::POP: return fill(opcode_stream, *this, g_table.find("POP")->second);
    case asmcode::PUSH: return fill(opcode_stream, *this, g_table.find("PUSH")->second);
    case asmcode::RET: return fill(opcode_stream, *this, g_table.find("RET")->second);   
    case asmcode::SAL: return fill(opcode_stream, *this, g_table.find("SAL")->second);
    case asmcode::SAR: return fill(opcode_stream, *this, g_table.find("SAR")->second);
    case asmcode::SETE: return fill(opcode_stream, *this, g_table.find("SETE")->second);
    case asmcode::SETNE: return fill(opcode_stream, *this, g_table.find("SETNE")->second);
    case asmcode::SETL: return fill(opcode_stream, *this, g_table.find("SETL")->second);
    case asmcode::SETG: return fill(opcode_stream, *this, g_table.find("SETG")->second);
    case asmcode::SETLE: return fill(opcode_stream, *this, g_table.find("SETLE")->second);
    case asmcode::SETGE: return fill(opcode_stream, *this, g_table.find("SETGE")->second);
    case asmcode::SHL: return fill(opcode_stream, *this, g_table.find("SHL")->second);  
    case asmcode::SHR: return fill(opcode_stream, *this, g_table.find("SHR")->second);
    case asmcode::SQRTPD: return fill(opcode_stream, *this, g_table.find("SQRTPD")->second); 
    case asmcode::SUB: return fill(opcode_stream, *this, g_table.find("SUB")->second);
    case asmcode::SUBSD: return fill(opcode_stream, *this, g_table.find("SUBSD")->second);    
    case asmcode::TEST: return fill(opcode_stream, *this, g_table.find("TEST")->second);
    case asmcode::UCOMISD: return fill(opcode_stream, *this, g_table.find("UCOMISD")->second);    
    case asmcode::XOR: return fill(opcode_stream, *this, g_table.find("XOR")->second);
    case asmcode::XORPD: return fill(opcode_stream, *this, g_table.find("XORPD")->second);
    default: throw std::logic_error("fill_opcode: this operation is not implemented");
    }
  //return 0;
  }

std::string asmcode::operation_to_string(operation oper)
  {
  switch (oper)
    {
    case asmcode::ADD: return std::string("ADD");
    case asmcode::ADDSD: return std::string("ADDSD");
    case asmcode::AND: return std::string("AND");
    case asmcode::CALL:return std::string("CALL");
    case asmcode::CALLEXTERNAL:return std::string("CALLEXTERNAL");
    case asmcode::COMMENT: return std::string("COMMENT");
    case asmcode::CMP: return std::string("CMP");
    case asmcode::CMPEQPD: return std::string("CMPEQPD");
    case asmcode::CMPLTPD: return std::string("CMPLTPD");
    case asmcode::CMPLEPD: return std::string("CMPLEPD");
    case asmcode::CQO: return std::string("CQO");
    case asmcode::CVTSI2SD: return std::string("CVTSI2SD");    
    case asmcode::CVTTSD2SI:return std::string("CVTTSD2SI");
    case asmcode::DEC: return std::string("DEC");
    case asmcode::DIV: return std::string("DIV");
    case asmcode::DIVSD: return std::string("DIVSD");   
    case asmcode::EXTERN: return std::string("EXTERN");
    case asmcode::F2XM1:return std::string("F2XM1");
    case asmcode::FADD: return std::string("FADD");   
    case asmcode::FISTPQ:return std::string("FISTPQ");  
    case asmcode::FILD:return std::string("FILD");
    case asmcode::FLD: return std::string("FLD");
    case asmcode::FLD1:return std::string("FLD1");  
    case asmcode::FLDPI: return std::string("FLDPI");
    case asmcode::FLDLN2:return std::string("FLDLN2"); 
    case asmcode::FMUL:return std::string("FMUL");
    case asmcode::FSIN:return std::string("FSIN");
    case asmcode::FCOS:return std::string("FCOS");
    case asmcode::FPATAN:return std::string("FPATAN");
    case asmcode::FPTAN: return std::string("FPTAN");
    case asmcode::FRNDINT:return std::string("FRNDINT");
    case asmcode::FSCALE: return std::string("FSCALE");
    case asmcode::FSQRT:return std::string("FSQRT");
    case asmcode::FSTP: return std::string("FSTP");
    case asmcode::FSUB: return std::string("FSUB");
    case asmcode::FSUBRP:return std::string("FSUBRP");
    case asmcode::FXCH: return std::string("FXCH");
    case asmcode::FYL2X: return std::string("FYL2X");
    case asmcode::GLOBAL:return std::string("GLOBAL");
    case asmcode::LABEL:return std::string("LABEL");
    case asmcode::LABEL_ALIGNED:return std::string("LABEL_ALIGNED");
    case asmcode::IDIV:return std::string("IDIV");
    case asmcode::IMUL:return std::string("IMUL");
    case asmcode::INC: return std::string("INC");
    case asmcode::JE: return std::string("JE");
    case asmcode::JL: return std::string("JL");
    case asmcode::JLE:return std::string("JLE");
    case asmcode::JA: return std::string("JA");
    case asmcode::JB: return std::string("JB");
    case asmcode::JG: return std::string("JG");
    case asmcode::JGE:return std::string("JGE");
    case asmcode::JNE:return std::string("JNE");
    case asmcode::JMP:return std::string("JMP");
    case asmcode::JES:return std::string("JES");
    case asmcode::JLS:return std::string("JLS");
    case asmcode::JLES:return std::string("JLES");
    case asmcode::JAS: return std::string("JAS");
    case asmcode::JBS: return std::string("JBS");
    case asmcode::JGS: return std::string("JGS");
    case asmcode::JGES:return std::string("JGES");
    case asmcode::JNES:return std::string("JNES");
    case asmcode::JMPS:return std::string("JMPS");
    case asmcode::MOV: return std::string("MOV");
    case asmcode::MOVQ: return std::string("MOVQ");
    case asmcode::MOVMSKPD: return std::string("MOVMSKPD");
    case asmcode::MOVSD:return std::string("MOVSD");
    case asmcode::MOVZX:return std::string("MOVZX");
    case asmcode::MUL: return std::string("MUL");
    case asmcode::MULSD:return std::string("MULSD");
    case asmcode::NEG:return std::string("NEG");
    case asmcode::NOP:return std::string("NOP");
    case asmcode::OR:return std::string("OR");
    case asmcode::POP: return std::string("POP");
    case asmcode::PUSH: return std::string("PUSH");
    case asmcode::RET: return std::string("RET");
    case asmcode::SAL: return std::string("SAL");
    case asmcode::SAR: return std::string("SAR");
    case asmcode::SETE: return std::string("SETE");
    case asmcode::SETNE:return std::string("SETNE");
    case asmcode::SETL: return std::string("SETL");
    case asmcode::SETG: return std::string("SETG");
    case asmcode::SETLE:return std::string("SETLE");
    case asmcode::SETGE:return std::string("SETGE");
    case asmcode::SHL: return std::string("SHL");
    case asmcode::SHR:return std::string("SHR");
    case asmcode::SQRTPD:return std::string("SQRTPD");
    case asmcode::SUB: return std::string("SUB");
    case asmcode::SUBSD: return std::string("SUBSD");
    case asmcode::TEST: return std::string("TEST");
    case asmcode::UCOMISD:return std::string("UCOMISD");    
    case asmcode::XOR: return std::string("XOR");
    case asmcode::XORPD: return std::string("XORPD");
    }
  return std::string();
  }

std::string asmcode::operand_to_string(operand op)
  {
  switch (op)
    {
    case asmcode::EMPTY: return std::string("EMPTY");
    case asmcode::AL: return std::string("AL");
    case asmcode::AH: return std::string("AH");
    case asmcode::BL: return std::string("BL");
    case asmcode::BH: return std::string("BH");
    case asmcode::CL: return std::string("CL");
    case asmcode::CH: return std::string("CH");
    case asmcode::DL: return std::string("DL");
    case asmcode::DH: return std::string("DH");   
    case asmcode::RAX: return std::string("RAX");
    case asmcode::RBX: return std::string("RBX");
    case asmcode::RCX: return std::string("RCX");
    case asmcode::RDX: return std::string("RDX");
    case asmcode::RDI: return std::string("RDI");
    case asmcode::RSI: return std::string("RSI");
    case asmcode::RSP: return std::string("RSP");
    case asmcode::RBP: return std::string("RBP");
    case asmcode::R8:  return std::string("R8");
    case asmcode::R9:  return std::string("R9");
    case asmcode::R10: return std::string("R10");
    case asmcode::R11: return std::string("R11");
    case asmcode::R12: return std::string("R12");
    case asmcode::R13: return std::string("R13");
    case asmcode::R14: return std::string("R14");
    case asmcode::R15: return std::string("R15");  
    case asmcode::MEM_RAX: return std::string("MEM_RAX");
    case asmcode::MEM_RBX: return std::string("MEM_RBX");
    case asmcode::MEM_RCX: return std::string("MEM_RCX");
    case asmcode::MEM_RDX: return std::string("MEM_RDX");
    case asmcode::MEM_RDI: return std::string("MEM_RDI");
    case asmcode::MEM_RSI: return std::string("MEM_RSI");
    case asmcode::MEM_RSP: return std::string("MEM_RSP");
    case asmcode::MEM_RBP: return std::string("MEM_RBP");
    case asmcode::MEM_R8:  return std::string("MEM_R8");
    case asmcode::MEM_R9:  return std::string("MEM_R9");
    case asmcode::MEM_R10: return std::string("MEM_R10");
    case asmcode::MEM_R11: return std::string("MEM_R11");
    case asmcode::MEM_R12: return std::string("MEM_R12");
    case asmcode::MEM_R13: return std::string("MEM_R13");
    case asmcode::MEM_R14: return std::string("MEM_R14");
    case asmcode::MEM_R15: return std::string("MEM_R15");
    case asmcode::BYTE_MEM_RAX: return std::string("BYTE_MEM_RAX");
    case asmcode::BYTE_MEM_RBX: return std::string("BYTE_MEM_RBX");
    case asmcode::BYTE_MEM_RCX: return std::string("BYTE_MEM_RCX");
    case asmcode::BYTE_MEM_RDX: return std::string("BYTE_MEM_RDX");
    case asmcode::BYTE_MEM_RDI: return std::string("BYTE_MEM_RDI");
    case asmcode::BYTE_MEM_RSI: return std::string("BYTE_MEM_RSI");
    case asmcode::BYTE_MEM_RSP: return std::string("BYTE_MEM_RSP");
    case asmcode::BYTE_MEM_RBP: return std::string("BYTE_MEM_RBP");
    case asmcode::BYTE_MEM_R8:  return std::string("BYTE_MEM_R8");
    case asmcode::BYTE_MEM_R9:  return std::string("BYTE_MEM_R9");
    case asmcode::BYTE_MEM_R10: return std::string("BYTE_MEM_R10");
    case asmcode::BYTE_MEM_R11: return std::string("BYTE_MEM_R11");
    case asmcode::BYTE_MEM_R12: return std::string("BYTE_MEM_R12");
    case asmcode::BYTE_MEM_R13: return std::string("BYTE_MEM_R13");
    case asmcode::BYTE_MEM_R14: return std::string("BYTE_MEM_R14");
    case asmcode::BYTE_MEM_R15: return std::string("BYTE_MEM_R15");  
    case asmcode::NUMBER: return std::string("NUMBER");
    case asmcode::ST0:  return std::string("ST0");
    case asmcode::ST1:  return std::string("ST1");
    case asmcode::ST2:  return std::string("ST2");
    case asmcode::ST3:  return std::string("ST3");
    case asmcode::ST4:  return std::string("ST4");
    case asmcode::ST5:  return std::string("ST5");
    case asmcode::ST6:  return std::string("ST6");
    case asmcode::ST7:  return std::string("ST7");
    case asmcode::XMM0: return std::string("XMM0");
    case asmcode::XMM1: return std::string("XMM1");
    case asmcode::XMM2: return std::string("XMM2");
    case asmcode::XMM3: return std::string("XMM3");
    case asmcode::XMM4: return std::string("XMM4");
    case asmcode::XMM5: return std::string("XMM5");
    case asmcode::XMM6: return std::string("XMM6");
    case asmcode::XMM7: return std::string("XMM7");
    case asmcode::XMM8: return std::string("XMM8");
    case asmcode::XMM9: return std::string("XMM9");
    case asmcode::XMM10:return std::string("XMM10");
    case asmcode::XMM11:return std::string("XMM11");
    case asmcode::XMM12:return std::string("XMM12");
    case asmcode::XMM13:return std::string("XMM13");
    case asmcode::XMM14:return std::string("XMM14");
    case asmcode::XMM15:return std::string("XMM15");   
    case asmcode::LABELADDRESS: return std::string("LABELADDRESS");
    }
  return std::string();
  }

ASM_END
