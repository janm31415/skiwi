#include "vm.h"

#include <iostream>
#include <sstream>
#include <cassert>

ASM_BEGIN

namespace
  {

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

  int number_of_operands(const asmcode::operation& op)
    {
    switch (op)
      {
      case asmcode::ADD: return 2;
      case asmcode::ADDSD: return 2;
      case asmcode::AND: return 2;
      case asmcode::CALL:return 1;
      case asmcode::COMMENT: return 0;
      case asmcode::CMP: return 2;
      case asmcode::CMPEQPD: return 2;
      case asmcode::CMPLTPD: return 2;
      case asmcode::CMPLEPD: return 2;
      case asmcode::CQO: return 0;
      case asmcode::CVTSI2SD: return 2;
      case asmcode::CVTTSD2SI:return 2;
      case asmcode::DEC: return 1;
      case asmcode::DIV: return 1;
      case asmcode::DIVSD: return 2;
      case asmcode::EXTERN: return 0;
      case asmcode::F2XM1:return 0;
      case asmcode::FADD: return 0;
      case asmcode::FISTPQ: return 1;
      case asmcode::FILD:return 1;
      case asmcode::FLD: return 1;
      case asmcode::FLD1:return 0;
      case asmcode::FLDPI: return 0;
      case asmcode::FLDLN2:return 0;
      case asmcode::FMUL:return 2;
      case asmcode::FSIN:return 0;
      case asmcode::FCOS:return 0;
      case asmcode::FPATAN:return 0;
      case asmcode::FPTAN: return 0;
      case asmcode::FRNDINT:return 0;
      case asmcode::FSCALE: return 0;
      case asmcode::FSQRT:return 0;
      case asmcode::FSTP: return 1;
      case asmcode::FSUB: return 0;
      case asmcode::FSUBRP:return 0;
      case asmcode::FXCH: return 0;
      case asmcode::FYL2X: return 0;
      case asmcode::GLOBAL:return 0;
      case asmcode::LABEL:return 0;
      case asmcode::LABEL_ALIGNED:return 0;
      case asmcode::IDIV:return 1;
      case asmcode::IMUL:return 1;
      case asmcode::INC: return 1;
      case asmcode::JE: return 1;
      case asmcode::JL: return 1;
      case asmcode::JLE:return 1;
      case asmcode::JA: return 1;
      case asmcode::JB: return 1;
      case asmcode::JG: return 1;
      case asmcode::JGE:return 1;
      case asmcode::JNE:return 1;
      case asmcode::JMP:return 1;
      case asmcode::JES:return 1;
      case asmcode::JLS:return 1;
      case asmcode::JLES:return 1;
      case asmcode::JAS: return 1;
      case asmcode::JBS: return 1;
      case asmcode::JGS: return 1;
      case asmcode::JGES:return 1;
      case asmcode::JNES:return 1;
      case asmcode::JMPS:return 1;
      case asmcode::MOV: return 2;
      case asmcode::MOVQ: return 2;
      case asmcode::MOVMSKPD: return 2;
      case asmcode::MOVSD:return 2;
      case asmcode::MOVZX:return 2;
      case asmcode::MUL: return 1;
      case asmcode::MULSD:return 2;
      case asmcode::NEG:return 1;
      case asmcode::NOP:return 0;
      case asmcode::OR:return 2;
      case asmcode::POP: return 1;
      case asmcode::PUSH: return 1;
      case asmcode::RET: return 0;
      case asmcode::SAL: return 2;
      case asmcode::SAR: return 2;
      case asmcode::SETE: return 1;
      case asmcode::SETNE:return 1;
      case asmcode::SETL: return 1;
      case asmcode::SETG: return 1;
      case asmcode::SETLE:return 1;
      case asmcode::SETGE:return 1;
      case asmcode::SHL: return 2;
      case asmcode::SHR:return 2;
      case asmcode::SQRTPD:return 2;
      case asmcode::SUB: return 2;
      case asmcode::SUBSD: return 2;
      case asmcode::TEST: return 2;
      case asmcode::UCOMISD: return 2;
      case asmcode::XOR: return 2;
      case asmcode::XORPD: return 2;
      default:
      {
      std::stringstream str;
      str << asmcode::operation_to_string(op) << " number of operands unknown!";
      throw std::logic_error(str.str());
      }
      }
    }

  enum operand_immediate_type
    {
    _VARIABLE,
    _8BIT,
    _32BIT,
    _64BIT
    };

  operand_immediate_type get_operand_immediate_type(const asmcode::operation& op)
    {
    switch (op)
      {
      case asmcode::CALL: return _32BIT;
      case asmcode::JE: return _32BIT;
      case asmcode::JL: return _32BIT;
      case asmcode::JLE:return _32BIT;
      case asmcode::JA: return _32BIT;
      case asmcode::JB: return _32BIT;
      case asmcode::JG: return _32BIT;
      case asmcode::JGE:return _32BIT;
      case asmcode::JNE:return _32BIT;
      case asmcode::JMP:return _32BIT;
      case asmcode::JES: return _8BIT;
      case asmcode::JLS: return _8BIT;
      case asmcode::JLES: return _8BIT;
      case asmcode::JAS: return _8BIT;
      case asmcode::JBS: return _8BIT;
      case asmcode::JGS: return _8BIT;
      case asmcode::JGES: return _8BIT;
      case asmcode::JNES: return _8BIT;
      case asmcode::JMPS: return _8BIT;
      default: return _VARIABLE;
      }
    }

  bool ignore_operation_as_bytecode(const asmcode::operation& op)
    {
    switch (op)
      {
      case asmcode::LABEL: return true;
      case asmcode::LABEL_ALIGNED: return true;
      case asmcode::GLOBAL: return true;
      case asmcode::COMMENT: return true;
      default: return false;
      }
    }

  void get_memory_size_type(uint8_t& opmem, bool& save_mem_size, const asmcode::operation& op, const asmcode::operand& oprnd, uint64_t oprnd_mem)
    {
    save_mem_size = false;
    opmem = 0;
    switch (oprnd)
      {
      case  asmcode::EMPTY:
      case  asmcode::AL:
      case  asmcode::AH:
      case  asmcode::BL:
      case  asmcode::BH:
      case  asmcode::CL:
      case  asmcode::CH:
      case  asmcode::DL:
      case  asmcode::DH:
      case  asmcode::RAX:
      case  asmcode::RBX:
      case  asmcode::RCX:
      case  asmcode::RDX:
      case  asmcode::RSI:
      case  asmcode::RDI:
      case  asmcode::RSP:
      case  asmcode::RBP:
      case  asmcode::R8:
      case  asmcode::R9:
      case  asmcode::R10:
      case  asmcode::R11:
      case  asmcode::R12:
      case  asmcode::R13:
      case  asmcode::R14:
      case  asmcode::R15:
      case  asmcode::ST0:
      case  asmcode::ST1:
      case  asmcode::ST2:
      case  asmcode::ST3:
      case  asmcode::ST4:
      case  asmcode::ST5:
      case  asmcode::ST6:
      case  asmcode::ST7:
      case  asmcode::XMM0:
      case  asmcode::XMM1:
      case  asmcode::XMM2:
      case  asmcode::XMM3:
      case  asmcode::XMM4:
      case  asmcode::XMM5:
      case  asmcode::XMM6:
      case  asmcode::XMM7:
      case  asmcode::XMM8:
      case  asmcode::XMM9:
      case  asmcode::XMM10:
      case  asmcode::XMM11:
      case  asmcode::XMM12:
      case  asmcode::XMM13:
      case  asmcode::XMM14:
      case  asmcode::XMM15:
        return;
      case asmcode::NUMBER:
      {
      auto memtype = get_operand_immediate_type(op);
      switch (memtype)
        {
        case _VARIABLE: break;
        case _8BIT: opmem = 1; return;
        case _32BIT: opmem = 3; return;
        case _64BIT: opmem = 4; return;
        }
      }
      default: break;
      } 
    if (is_8_bit(oprnd_mem))
      opmem = 1;
    else if (is_16_bit(oprnd_mem))
      {
      save_mem_size = true;
      opmem = 2;
      }
    else if (is_32_bit(oprnd_mem))
      {
      save_mem_size = true;
      opmem = 3;
      }
    else
      {
      save_mem_size = true;
      opmem = 4;
      }      
    if (oprnd == asmcode::LABELADDRESS)
      {
      save_mem_size = true;
      opmem = 4;
      }
    }

  /*
  byte 1: operation opcode: equal to (int)asmcode::operation value of instr.oper
  byte 2: first operand: equal to (int)asmcode::operand of instr.operand1
  byte 3: second operand: equal to (int)asmcode::operand of instr.operand2
  byte 4: information on operand1_mem and operand2_mem
          First four bits equal to: 0 => instr.operand1_mem equals zero
                                  : 1 => instr.operand1_mem needs 8 bits
                                  : 2 => instr.operand1_mem needs 16 bits
                                  : 3 => instr.operand1_mem needs 32 bits
                                  : 4 => instr.operand1_mem needs 64 bits
          Last four bits equal to : 0 => instr.operand2_mem equals zero
                                  : 1 => instr.operand2_mem needs 8 bits
                                  : 2 => instr.operand2_mem needs 16 bits
                                  : 3 => instr.operand2_mem needs 32 bits
                                  : 4 => instr.operand2_mem needs 64 bits
  byte 5+: instr.operand1_mem using as many bytes as warranted by byte 4, followed by instr.operand2_mem using as many bytes as warranted by byte4.
  */
  uint64_t fill_vm_bytecode(const asmcode::instruction& instr, uint8_t* opcode_stream)
    {
    uint64_t sz = 0;
    if (ignore_operation_as_bytecode(instr.oper))
      return sz;
    opcode_stream[sz++] = (uint8_t)instr.oper;
    uint8_t op1mem = 0;
    uint8_t op2mem = 0;
    int nr_ops = number_of_operands(instr.oper);
    if (nr_ops == 1)
      {
      bool savemem = true;
      get_memory_size_type(op1mem, savemem, instr.oper, instr.operand1, instr.operand1_mem);
      if (savemem)
        {
        opcode_stream[sz++] = (uint8_t)instr.operand1;
        opcode_stream[sz++] = op1mem;
        }
      else
        {
        opcode_stream[sz++] = (uint8_t)instr.operand1 | operand_has_8bit_mem;
        }
      }
    else if (nr_ops == 2)
      {
      bool savemem1 = true;
      bool savemem2 = true;
      get_memory_size_type(op1mem, savemem1, instr.oper, instr.operand1, instr.operand1_mem);
      get_memory_size_type(op2mem, savemem2, instr.oper, instr.operand2, instr.operand2_mem);
      if (savemem1 || savemem2)
        {
        opcode_stream[sz++] = (uint8_t)instr.operand1;
        opcode_stream[sz++] = (uint8_t)instr.operand2;
        opcode_stream[sz++] = (uint8_t)(op2mem << 4) | op1mem;
        }
      else
        {
        opcode_stream[sz++] = (uint8_t)instr.operand1 | operand_has_8bit_mem;
        opcode_stream[sz++] = (uint8_t)instr.operand2 | operand_has_8bit_mem;
        }
      }
    switch (op1mem)
      {
      case 1: opcode_stream[sz++] = (uint8_t)instr.operand1_mem; break;
      case 2: *(reinterpret_cast<uint16_t*>(opcode_stream + sz)) = (uint16_t)instr.operand1_mem; sz += 2; break;
      case 3: *(reinterpret_cast<uint32_t*>(opcode_stream + sz)) = (uint32_t)instr.operand1_mem; sz += 4; break;
      case 4: *(reinterpret_cast<uint64_t*>(opcode_stream + sz)) = (uint64_t)instr.operand1_mem; sz += 8; break;
      default: break;
      }
    switch (op2mem)
      {
      case 1: opcode_stream[sz++] = (uint8_t)instr.operand2_mem; break;
      case 2: *(reinterpret_cast<uint16_t*>(opcode_stream + sz)) = (uint16_t)instr.operand2_mem; sz += 2; break;
      case 3: *(reinterpret_cast<uint32_t*>(opcode_stream + sz)) = (uint32_t)instr.operand2_mem; sz += 4; break;
      case 4: *(reinterpret_cast<uint64_t*>(opcode_stream + sz)) = (uint64_t)instr.operand2_mem; sz += 8; break;
      default: break;
      }
    return sz;
    /*
    switch (instr.oper)
      {
      case asmcode::LABEL: return 0;
      case asmcode::LABEL_ALIGNED: return 0;
      case asmcode::GLOBAL: return 0;
      case asmcode::COMMENT: return 0;
      case asmcode::NOP:
      {
      opcode_stream[0] = (uint8_t)instr.oper;
      return 1;
      }
      default: break;
      }
    opcode_stream[0] = (uint8_t)instr.oper;
    opcode_stream[1] = (uint8_t)instr.operand1;
    opcode_stream[2] = (uint8_t)instr.operand2;
    uint8_t op1mem = 0;
    uint8_t op2mem = 0;
    if (instr.operand1_mem != 0)
      {
      if (is_8_bit(instr.operand1_mem))
        op1mem = 1;
      else if (is_16_bit(instr.operand1_mem))
        op1mem = 2;
      else if (is_32_bit(instr.operand1_mem))
        op1mem = 3;
      else
        op1mem = 4;
      }
    if (op1mem < 4)
      {
      if (instr.operand1 == asmcode::LABELADDRESS)
        op1mem = 4;
      }
    if (op1mem < 3)
      {
      if (instr.oper == asmcode::CALL)
        op1mem = 3;
      if (instr.oper == asmcode::JMP)
        op1mem = 3;
      if (instr.oper == asmcode::JA)
        op1mem = 3;
      if (instr.oper == asmcode::JB)
        op1mem = 3;
      if (instr.oper == asmcode::JE)
        op1mem = 3;
      if (instr.oper == asmcode::JL)
        op1mem = 3;
      if (instr.oper == asmcode::JLE)
        op1mem = 3;
      if (instr.oper == asmcode::JG)
        op1mem = 3;
      if (instr.oper == asmcode::JGE)
        op1mem = 3;
      if (instr.oper == asmcode::JNE)
        op1mem = 3;
      }
    if (instr.operand2_mem != 0)
      {
      if (is_8_bit(instr.operand2_mem))
        op2mem = 1;
      else if (is_16_bit(instr.operand2_mem))
        op2mem = 2;
      else if (is_32_bit(instr.operand2_mem))
        op2mem = 3;
      else
        op2mem = 4;
      }
    if (op2mem < 4)
      {
      if (instr.operand2 == asmcode::LABELADDRESS)
        op2mem = 4;
      }
    opcode_stream[3] = (uint8_t)(op2mem << 4) | op1mem;
    uint64_t sz = 4;
    switch (op1mem)
      {
      case 1: opcode_stream[sz++] = (uint8_t)instr.operand1_mem; break;
      case 2: *(reinterpret_cast<uint16_t*>(opcode_stream + sz)) = (uint16_t)instr.operand1_mem; sz += 2; break;
      case 3: *(reinterpret_cast<uint32_t*>(opcode_stream + sz)) = (uint32_t)instr.operand1_mem; sz += 4; break;
      case 4: *(reinterpret_cast<uint64_t*>(opcode_stream + sz)) = (uint64_t)instr.operand1_mem; sz += 8; break;
      default: break;
      }
    switch (op2mem)
      {
      case 1: opcode_stream[sz++] = (uint8_t)instr.operand2_mem; break;
      case 2: *(reinterpret_cast<uint16_t*>(opcode_stream + sz)) = (uint16_t)instr.operand2_mem; sz += 2; break;
      case 3: *(reinterpret_cast<uint32_t*>(opcode_stream + sz)) = (uint32_t)instr.operand2_mem; sz += 4; break;
      case 4: *(reinterpret_cast<uint64_t*>(opcode_stream + sz)) = (uint64_t)instr.operand2_mem; sz += 8; break;
      default: break;
      }

    return sz;
    */
    }


  void first_pass(first_pass_data& data, asmcode& code, const std::map<std::string, uint64_t>& externals)
    {
    uint8_t buffer[255];
    data.size = 0;
    data.label_to_address.clear();
    for (auto it = code.get_instructions_list().begin(); it != code.get_instructions_list().end(); ++it)
      {
      std::vector<std::pair<size_t, int>> nops_to_add;
      for (size_t i = 0; i < it->size(); ++i)
        {
        auto instr = (*it)[i];
        switch (instr.oper)
          {
          case asmcode::CALL:
          {
          auto it2 = externals.find(instr.text);
          if (it2 != externals.end())
            {
            instr.oper = asmcode::MOV;
            instr.operand1 = asmcode::RAX;
            instr.operand2 = asmcode::NUMBER;
            instr.operand2_mem = it2->second;
            data.size += fill_vm_bytecode(instr, buffer);
            instr.oper = asmcode::CALL;
            instr.operand1 = asmcode::RAX;
            instr.operand2 = asmcode::EMPTY;
            data.size += fill_vm_bytecode(instr, buffer);
            }
          else
            {
            if (instr.operand1 == asmcode::EMPTY)
              {
              instr.operand1 = asmcode::NUMBER;
              instr.operand1_mem = 0x11111111;
              }
            data.size += fill_vm_bytecode(instr, buffer);
            }
          break;
          }
          case asmcode::JMP:
          case asmcode::JE:
          case asmcode::JL:
          case asmcode::JLE:
          case asmcode::JG:
          case asmcode::JA:
          case asmcode::JB:
          case asmcode::JGE:
          case asmcode::JNE:
          {
          if (instr.operand1 == asmcode::EMPTY)
            {
            instr.operand1 = asmcode::NUMBER;
            instr.operand1_mem = 0x11111111;
            }
          data.size += fill_vm_bytecode(instr, buffer);
          break;
          }
          case asmcode::JMPS:
          case asmcode::JES:
          case asmcode::JLS:
          case asmcode::JLES:
          case asmcode::JGS:
          case asmcode::JAS:
          case asmcode::JBS:
          case asmcode::JGES:
          case asmcode::JNES:
          {
          instr.operand1 = asmcode::NUMBER;
          instr.operand1_mem = 0x11;
          data.size += fill_vm_bytecode(instr, buffer);
          break;
          }
          case asmcode::LABEL:
            data.label_to_address[instr.text] = data.size; break;
          case asmcode::LABEL_ALIGNED:
            if (data.size & 7)
              {
              int nr_of_nops = 8 - (data.size & 7);
              data.size += nr_of_nops;
              nops_to_add.emplace_back(i, nr_of_nops);
              }
            data.label_to_address[instr.text] = data.size; break;
          case asmcode::GLOBAL:
            if (data.size & 7)
              {
              int nr_of_nops = 8 - (data.size & 7);
              data.size += nr_of_nops;
              nops_to_add.emplace_back(i, nr_of_nops);
              }
            data.label_to_address[instr.text] = data.size; break;
          case asmcode::EXTERN:
          {
          auto it2 = externals.find(instr.text);
          if (it2 == externals.end())
            throw std::logic_error("error: external is not defined");
          data.external_to_address[instr.text] = it2->second;
          break;
          }
          default:
            data.size += fill_vm_bytecode(instr, buffer); break;
          }
        }
      size_t nops_offset = 0;
      for (auto nops : nops_to_add)
        {
        std::vector<asmcode::instruction> nops_instructions(nops.second, asmcode::instruction(asmcode::NOP));
        it->insert(it->begin() + nops_offset + nops.first, nops_instructions.begin(), nops_instructions.end());
        nops_offset += nops.second;
        }
      }
    }


  uint8_t* second_pass(uint8_t* func, const first_pass_data& data, const asmcode& code)
    {
    uint64_t address_start = (uint64_t)(reinterpret_cast<uint64_t*>(func));
    uint8_t* start = func;
    for (auto it = code.get_instructions_list().begin(); it != code.get_instructions_list().end(); ++it)
      {
      for (asmcode::instruction instr : *it)
        {
        if (instr.operand1 == asmcode::LABELADDRESS)
          {
          auto it2 = data.label_to_address.find(instr.text);
          if (it2 == data.label_to_address.end())
            throw std::logic_error("error: label is not defined");
          instr.operand1_mem = address_start + it2->second;
          }
        if (instr.operand2 == asmcode::LABELADDRESS)
          {
          auto it2 = data.label_to_address.find(instr.text);
          if (it2 == data.label_to_address.end())
            throw std::logic_error("error: label is not defined");
          instr.operand2_mem = address_start + it2->second;
          }
        switch (instr.oper)
          {
          case asmcode::CALL:
          {
          if (instr.operand1 != asmcode::EMPTY)
            break;
          auto it_label = data.label_to_address.find(instr.text);
          auto it_external = data.external_to_address.find(instr.text);
          if (it_external != data.external_to_address.end())
            {
            asmcode::instruction extra_instr(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, it_external->second);
            func += fill_vm_bytecode(extra_instr, func);
            instr.operand1 = asmcode::RAX;
            }
          else if (it_label != data.label_to_address.end())
            {
            int64_t address = (int64_t)it_label->second;
            int64_t current = (int64_t)(func - start);
            instr.operand1 = asmcode::NUMBER;
            instr.operand1_mem = (int64_t(address - current));
            }
          else
            throw std::logic_error("second_pass error: call target does not exist");
          break;
          }
          case asmcode::JE:
          case asmcode::JL:
          case asmcode::JLE:
          case asmcode::JG:
          case asmcode::JA:
          case asmcode::JB:
          case asmcode::JGE:
          case asmcode::JNE:
          {
          if (instr.operand1 != asmcode::EMPTY)
            break;
          auto it2 = data.label_to_address.find(instr.text);
          if (it2 == data.label_to_address.end())
            throw std::logic_error("second_pass error: label does not exist");
          int64_t address = (int64_t)it2->second;
          int64_t current = (int64_t)(func - start);
          instr.operand1 = asmcode::NUMBER;
          instr.operand1_mem = (int64_t(address - current));
          break;
          }
          case asmcode::JMP:
          {
          if (instr.operand1 != asmcode::EMPTY)
            break;
          auto it2 = data.label_to_address.find(instr.text);
          if (it2 == data.label_to_address.end())
            throw std::logic_error("second_pass error: label does not exist");
          int64_t address = (int64_t)it2->second;
          int64_t current = (int64_t)(func - start);
          instr.operand1 = asmcode::NUMBER;
          instr.operand1_mem = (int64_t(address - current));
          break;
          }
          case asmcode::JES:
          case asmcode::JLS:
          case asmcode::JLES:
          case asmcode::JGS:
          case asmcode::JAS:
          case asmcode::JBS:
          case asmcode::JGES:
          case asmcode::JNES:
          case asmcode::JMPS:
          {
          if (instr.operand1 != asmcode::EMPTY)
            break;
          auto it2 = data.label_to_address.find(instr.text);
          if (it2 == data.label_to_address.end())
            throw std::logic_error("second_pass error: label does not exist");
          int64_t address = (int64_t)it2->second;
          int64_t current = (int64_t)(func - start);
          instr.operand1 = asmcode::NUMBER;
          instr.operand1_mem = (int64_t(address - current));
          if ((int64_t)instr.operand1_mem > 127 || (int64_t)instr.operand1_mem < -128)
            throw std::logic_error("second_pass error: jump short is too far");
          break;
          }
          }
        func += fill_vm_bytecode(instr, func);
        }
      }
    while ((uint64_t)(func - start) < data.size)
      {
      asmcode::instruction dummy(asmcode::NOP);
      func += fill_vm_bytecode(dummy, func);
      }
    return func;
    }
  }

void* vm_bytecode(uint64_t& size, first_pass_data& d, asmcode& code, const std::map<std::string, uint64_t>& externals)
  {
  d.external_to_address.clear();
  d.label_to_address.clear();
  first_pass(d, code, externals);

  uint8_t* compiled_func = new uint8_t[d.size + d.data_size];

  if (!compiled_func)
    throw std::runtime_error("Could not allocate virtual memory");

  uint8_t* func_end = second_pass((uint8_t*)compiled_func, d, code);

  uint64_t size_used = func_end - (uint8_t*)compiled_func;

  if (size_used != d.size)
    {
    throw std::logic_error("error: error in size computation.");
    }

  size = d.size + d.data_size;

  return (void*)compiled_func;
  }

void* vm_bytecode(uint64_t& size, asmcode& code, const std::map<std::string, uint64_t>& externals)
  {
  first_pass_data d;
  return vm_bytecode(size, d, code, externals);
  }

void* vm_bytecode(uint64_t& size, first_pass_data& d, asmcode& code)
  {
  std::map<std::string, uint64_t> externals;
  return vm_bytecode(size, d, code, externals);
  }

void* vm_bytecode(uint64_t& size, asmcode& code)
  {
  std::map<std::string, uint64_t> externals;
  return vm_bytecode(size, code, externals);
  }

void free_bytecode(void* f, uint64_t size)
  {
  (void*)size;
  delete[] f;
  }


/*
byte 1: operation opcode: equal to (int)asmcode::operation value of instr.oper
byte 2: first operand: equal to (int)asmcode::operand of instr.operand1
byte 3: second operand: equal to (int)asmcode::operand of instr.operand2
byte 4: information on operand1_mem and operand2_mem
        First four bits equal to: 0 => instr.operand1_mem equals zero
                                : 1 => instr.operand1_mem needs 8 bits
                                : 2 => instr.operand1_mem needs 16 bits
                                : 3 => instr.operand1_mem needs 32 bits
                                : 4 => instr.operand1_mem needs 64 bits
        Last four bits equal to : 0 => instr.operand2_mem equals zero
                                : 1 => instr.operand2_mem needs 8 bits
                                : 2 => instr.operand2_mem needs 16 bits
                                : 3 => instr.operand2_mem needs 32 bits
                                : 4 => instr.operand2_mem needs 64 bits
byte 5+: instr.operand1_mem using as many bytes as warranted by byte 4, followed by instr.operand2_mem using as many bytes as warranted by byte4.

*/
uint64_t disassemble_bytecode(asmcode::operation& op,
  asmcode::operand& operand1,
  asmcode::operand& operand2,
  uint64_t& operand1_mem,
  uint64_t& operand2_mem,
  const uint8_t* bytecode)
  {
  operand1 = asmcode::EMPTY;
  operand2 = asmcode::EMPTY;
  operand1_mem = 0;
  operand2_mem = 0;
  uint64_t sz = 0;
  uint8_t op1mem = 0;
  uint8_t op2mem = 0;
  op = (asmcode::operation)bytecode[sz++];
  int nr_ops = number_of_operands(op);
  if (nr_ops == 0)
    return sz;
  if (nr_ops == 1)
    {
    uint8_t op1 = bytecode[sz++];
    //bool savemem = true;
    //get_memory_size_type(op1mem, savemem, op, operand1, 0);
    //if (savemem)
    if ((op1 & operand_has_8bit_mem) == 0)
      {
      op1mem = bytecode[sz++];
      operand1 = (asmcode::operand)op1;
      }
    else
      {
      op1 &= ~operand_has_8bit_mem;
      operand1 = (asmcode::operand)op1;
      bool savemem;
      get_memory_size_type(op1mem, savemem, op, operand1, 0);
      }    
    }
  else
    {
    assert(nr_ops == 2);
    uint8_t op1 = bytecode[sz++];
    uint8_t op2 = bytecode[sz++];
    //bool savemem1 = true;
    //get_memory_size_type(op1mem, savemem1, op, operand1, 0);
    //bool savemem2 = true;
    //get_memory_size_type(op2mem, savemem2, op, operand2, 0);
    //if (savemem1 || savemem2)
    if ((op1 & operand_has_8bit_mem) == 0)
      {
      op1mem = bytecode[sz] & 15;
      op2mem = bytecode[sz] >> 4;
      operand1 = (asmcode::operand)op1;
      operand2 = (asmcode::operand)op2;
      ++sz;
      }
    else
      {
      op1 &= ~operand_has_8bit_mem;
      op2 &= ~operand_has_8bit_mem;
      operand1 = (asmcode::operand)op1;
      operand2 = (asmcode::operand)op2;
      bool savemem;
      get_memory_size_type(op1mem, savemem, op, operand1, 0);
      get_memory_size_type(op2mem, savemem, op, operand2, 0);
      }
    }
  switch (op1mem)
    {
    case 1: operand1_mem = (int8_t)bytecode[sz++]; break;
    case 2: operand1_mem = (int16_t)(*reinterpret_cast<const uint16_t*>(bytecode + sz)); sz += 2; break;
    case 3: operand1_mem = (int32_t)(*reinterpret_cast<const uint32_t*>(bytecode + sz)); sz += 4; break;
    case 4: operand1_mem = *reinterpret_cast<const uint64_t*>(bytecode + sz); sz += 8; break;
    default: operand1_mem = 0; break;
    }
  switch (op2mem)
    {
    case 1: operand2_mem = (int8_t)bytecode[sz++]; break;
    case 2: operand2_mem = (int16_t)(*reinterpret_cast<const uint16_t*>(bytecode + sz)); sz += 2; break;
    case 3: operand2_mem = (int32_t)(*reinterpret_cast<const uint32_t*>(bytecode + sz)); sz += 4; break;
    case 4: operand2_mem = *reinterpret_cast<const uint64_t*>(bytecode + sz); sz += 8; break;
    default: operand2_mem = 0; break;
    }
  return sz;
  /*
  op = (asmcode::operation)bytecode[0];
  if (op == asmcode::NOP)
    return 1;
  operand1 = (asmcode::operand)bytecode[1];
  operand2 = (asmcode::operand)bytecode[2];
  uint8_t op1mem = bytecode[3] & 15;
  uint8_t op2mem = bytecode[3] >> 4;
  uint64_t sz = 4;
  switch (op1mem)
    {
    case 1: operand1_mem = (int8_t)bytecode[sz++]; break;
    case 2: operand1_mem = (int16_t)(*reinterpret_cast<const uint16_t*>(bytecode + sz)); sz += 2; break;
    case 3: operand1_mem = (int32_t)(*reinterpret_cast<const uint32_t*>(bytecode + sz)); sz += 4; break;
    case 4: operand1_mem = *reinterpret_cast<const uint64_t*>(bytecode + sz); sz += 8; break;
    default: operand1_mem = 0; break;
    }
  switch (op2mem)
    {
    case 1: operand2_mem = (int8_t)bytecode[sz++]; break;
    case 2: operand2_mem = (int16_t)(*reinterpret_cast<const uint16_t*>(bytecode+sz)); sz += 2; break;
    case 3: operand2_mem = (int32_t)(*reinterpret_cast<const uint32_t*>(bytecode+sz)); sz += 4; break;
    case 4: operand2_mem = *reinterpret_cast<const uint64_t*>(bytecode+sz); sz += 8; break;
    default: operand2_mem = 0; break;
    }
  return sz;
  */
  }

registers::registers()
  {
  rbp = (uint64_t)(&stack[0]);
  rsp = (uint64_t)(&stack[256]);
  eflags = 0;
  }

namespace
  {
  uint8_t* get_address_8bit(asmcode::operand oper, uint64_t operand_mem, registers& regs)
    {
    switch (oper)
      {
      case asmcode::EMPTY: return nullptr;
      case asmcode::AL: return (uint8_t*)&regs.rax;
      case asmcode::AH: return (uint8_t*)(((uint8_t*)(&regs.rax)) + 1);
      case asmcode::BL: return (uint8_t*)&regs.rbx;
      case asmcode::BH: return (uint8_t*)(((uint8_t*)(&regs.rbx)) + 1);
      case asmcode::CL: return (uint8_t*)&regs.rcx;
      case asmcode::CH: return (uint8_t*)(((uint8_t*)(&regs.rcx)) + 1);
      case asmcode::DL: return (uint8_t*)&regs.rdx;
      case asmcode::DH: return (uint8_t*)(((uint8_t*)(&regs.rdx)) + 1);
      case asmcode::RAX: return nullptr;
      case asmcode::RBX: return nullptr;
      case asmcode::RCX: return nullptr;
      case asmcode::RDX: return nullptr;
      case asmcode::RDI: return nullptr;
      case asmcode::RSI: return nullptr;
      case asmcode::RSP: return nullptr;
      case asmcode::RBP: return nullptr;
      case asmcode::R8:  return nullptr;
      case asmcode::R9:  return nullptr;
      case asmcode::R10: return nullptr;
      case asmcode::R11: return nullptr;
      case asmcode::R12: return nullptr;
      case asmcode::R13: return nullptr;
      case asmcode::R14: return nullptr;
      case asmcode::R15: return nullptr;
      case asmcode::MEM_RAX: return nullptr;
      case asmcode::MEM_RBX: return nullptr;
      case asmcode::MEM_RCX: return nullptr;
      case asmcode::MEM_RDX: return nullptr;
      case asmcode::MEM_RDI: return nullptr;
      case asmcode::MEM_RSI: return nullptr;
      case asmcode::MEM_RSP: return nullptr;
      case asmcode::MEM_RBP: return nullptr;
      case asmcode::MEM_R8:  return nullptr;
      case asmcode::MEM_R9:  return nullptr;
      case asmcode::MEM_R10: return nullptr;
      case asmcode::MEM_R11: return nullptr;
      case asmcode::MEM_R12: return nullptr;
      case asmcode::MEM_R13: return nullptr;
      case asmcode::MEM_R14: return nullptr;
      case asmcode::MEM_R15: return nullptr;
      case asmcode::BYTE_MEM_RAX: return (uint8_t*)(regs.rax + operand_mem);
      case asmcode::BYTE_MEM_RBX: return (uint8_t*)(regs.rbx + operand_mem);
      case asmcode::BYTE_MEM_RCX: return (uint8_t*)(regs.rcx + operand_mem);
      case asmcode::BYTE_MEM_RDX: return (uint8_t*)(regs.rdx + operand_mem);
      case asmcode::BYTE_MEM_RDI: return (uint8_t*)(regs.rdi + operand_mem);
      case asmcode::BYTE_MEM_RSI: return (uint8_t*)(regs.rsi + operand_mem);
      case asmcode::BYTE_MEM_RSP: return (uint8_t*)(regs.rsp + operand_mem);
      case asmcode::BYTE_MEM_RBP: return (uint8_t*)(regs.rbp + operand_mem);
      case asmcode::BYTE_MEM_R8:  return (uint8_t*)(regs.r8 + operand_mem);
      case asmcode::BYTE_MEM_R9:  return (uint8_t*)(regs.r9 + operand_mem);
      case asmcode::BYTE_MEM_R10: return (uint8_t*)(regs.r10 + operand_mem);
      case asmcode::BYTE_MEM_R11: return (uint8_t*)(regs.r11 + operand_mem);
      case asmcode::BYTE_MEM_R12: return (uint8_t*)(regs.r12 + operand_mem);
      case asmcode::BYTE_MEM_R13: return (uint8_t*)(regs.r13 + operand_mem);
      case asmcode::BYTE_MEM_R14: return (uint8_t*)(regs.r14 + operand_mem);
      case asmcode::BYTE_MEM_R15: return (uint8_t*)(regs.r15 + operand_mem);
      case asmcode::NUMBER: return nullptr;
      case asmcode::ST0:  return nullptr;
      case asmcode::ST1:  return nullptr;
      case asmcode::ST2:  return nullptr;
      case asmcode::ST3:  return nullptr;
      case asmcode::ST4:  return nullptr;
      case asmcode::ST5:  return nullptr;
      case asmcode::ST6:  return nullptr;
      case asmcode::ST7:  return nullptr;
      case asmcode::XMM0: return nullptr;
      case asmcode::XMM1: return nullptr;
      case asmcode::XMM2: return nullptr;
      case asmcode::XMM3: return nullptr;
      case asmcode::XMM4: return nullptr;
      case asmcode::XMM5: return nullptr;
      case asmcode::XMM6: return nullptr;
      case asmcode::XMM7: return nullptr;
      case asmcode::XMM8: return nullptr;
      case asmcode::XMM9: return nullptr;
      case asmcode::XMM10:return nullptr;
      case asmcode::XMM11:return nullptr;
      case asmcode::XMM12:return nullptr;
      case asmcode::XMM13:return nullptr;
      case asmcode::XMM14:return nullptr;
      case asmcode::XMM15:return nullptr;
      case asmcode::LABELADDRESS: return nullptr;
      default: return nullptr;
      }
    }

  uint64_t* get_address_64bit(asmcode::operand oper, uint64_t operand_mem, registers& regs)
    {
    switch (oper)
      {
      case asmcode::EMPTY: return nullptr;
      case asmcode::AL: return nullptr;
      case asmcode::AH: return nullptr;
      case asmcode::BL: return nullptr;
      case asmcode::BH: return nullptr;
      case asmcode::CL: return nullptr;
      case asmcode::CH: return nullptr;
      case asmcode::DL: return nullptr;
      case asmcode::DH: return nullptr;
      case asmcode::RAX: return &regs.rax;
      case asmcode::RBX: return &regs.rbx;
      case asmcode::RCX: return &regs.rcx;
      case asmcode::RDX: return &regs.rdx;
      case asmcode::RDI: return &regs.rdi;
      case asmcode::RSI: return &regs.rsi;
      case asmcode::RSP: return &regs.rsp;
      case asmcode::RBP: return &regs.rbp;
      case asmcode::R8:  return &regs.r8;
      case asmcode::R9:  return &regs.r9;
      case asmcode::R10: return &regs.r10;
      case asmcode::R11: return &regs.r11;
      case asmcode::R12: return &regs.r12;
      case asmcode::R13: return &regs.r13;
      case asmcode::R14: return &regs.r14;
      case asmcode::R15: return &regs.r15;
      case asmcode::MEM_RAX: return (uint64_t*)(regs.rax + operand_mem);
      case asmcode::MEM_RBX: return (uint64_t*)(regs.rbx + operand_mem);
      case asmcode::MEM_RCX: return (uint64_t*)(regs.rcx + operand_mem);
      case asmcode::MEM_RDX: return (uint64_t*)(regs.rdx + operand_mem);
      case asmcode::MEM_RDI: return (uint64_t*)(regs.rdi + operand_mem);
      case asmcode::MEM_RSI: return (uint64_t*)(regs.rsi + operand_mem);
      case asmcode::MEM_RSP: return (uint64_t*)(regs.rsp + operand_mem);
      case asmcode::MEM_RBP: return (uint64_t*)(regs.rbp + operand_mem);
      case asmcode::MEM_R8:  return (uint64_t*)(regs.r8 + operand_mem);
      case asmcode::MEM_R9:  return (uint64_t*)(regs.r9 + operand_mem);
      case asmcode::MEM_R10: return (uint64_t*)(regs.r10 + operand_mem);
      case asmcode::MEM_R11: return (uint64_t*)(regs.r11 + operand_mem);
      case asmcode::MEM_R12: return (uint64_t*)(regs.r12 + operand_mem);
      case asmcode::MEM_R13: return (uint64_t*)(regs.r13 + operand_mem);
      case asmcode::MEM_R14: return (uint64_t*)(regs.r14 + operand_mem);
      case asmcode::MEM_R15: return (uint64_t*)(regs.r15 + operand_mem);
      case asmcode::BYTE_MEM_RAX: return nullptr;
      case asmcode::BYTE_MEM_RBX: return nullptr;
      case asmcode::BYTE_MEM_RCX: return nullptr;
      case asmcode::BYTE_MEM_RDX: return nullptr;
      case asmcode::BYTE_MEM_RDI: return nullptr;
      case asmcode::BYTE_MEM_RSI: return nullptr;
      case asmcode::BYTE_MEM_RSP: return nullptr;
      case asmcode::BYTE_MEM_RBP: return nullptr;
      case asmcode::BYTE_MEM_R8:  return nullptr;
      case asmcode::BYTE_MEM_R9:  return nullptr;
      case asmcode::BYTE_MEM_R10: return nullptr;
      case asmcode::BYTE_MEM_R11: return nullptr;
      case asmcode::BYTE_MEM_R12: return nullptr;
      case asmcode::BYTE_MEM_R13: return nullptr;
      case asmcode::BYTE_MEM_R14: return nullptr;
      case asmcode::BYTE_MEM_R15: return nullptr;
      case asmcode::NUMBER: return nullptr;
      case asmcode::ST0:  return nullptr;
      case asmcode::ST1:  return nullptr;
      case asmcode::ST2:  return nullptr;
      case asmcode::ST3:  return nullptr;
      case asmcode::ST4:  return nullptr;
      case asmcode::ST5:  return nullptr;
      case asmcode::ST6:  return nullptr;
      case asmcode::ST7:  return nullptr;
      case asmcode::XMM0: return (uint64_t*)(&regs.xmm0);
      case asmcode::XMM1: return (uint64_t*)(&regs.xmm1);
      case asmcode::XMM2: return (uint64_t*)(&regs.xmm2);
      case asmcode::XMM3: return (uint64_t*)(&regs.xmm3);
      case asmcode::XMM4: return (uint64_t*)(&regs.xmm4);
      case asmcode::XMM5: return (uint64_t*)(&regs.xmm5);
      case asmcode::XMM6: return (uint64_t*)(&regs.xmm6);
      case asmcode::XMM7: return (uint64_t*)(&regs.xmm7);
      case asmcode::XMM8: return (uint64_t*)(&regs.xmm8);
      case asmcode::XMM9: return (uint64_t*)(&regs.xmm9);
      case asmcode::XMM10:return (uint64_t*)(&regs.xmm10);
      case asmcode::XMM11:return (uint64_t*)(&regs.xmm11);
      case asmcode::XMM12:return (uint64_t*)(&regs.xmm12);
      case asmcode::XMM13:return (uint64_t*)(&regs.xmm13);
      case asmcode::XMM14:return (uint64_t*)(&regs.xmm14);
      case asmcode::XMM15:return (uint64_t*)(&regs.xmm15);
      case asmcode::LABELADDRESS: return nullptr;
      default: return nullptr;
      }
    }

  struct AddOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left += right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left += (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left += right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left += (uint8_t)right;
      }
    };

  struct AndOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left &= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left &= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left &= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left &= (uint8_t)right;
      }
    };

  struct MovOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left = right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left = (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left = right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left = (uint8_t)right;
      }
    };

  struct OrOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left |= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left |= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left |= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left |= (uint8_t)right;
      }
    };

  struct ShlOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left <<= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left <<= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left <<= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left <<= (uint8_t)right;
      }
    };

  struct SarOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      int64_t l = (int64_t)left;
      l >>= right;
      left = (uint64_t)l;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      int64_t l = (int64_t)left;
      l >>= (uint64_t)right;
      left = (uint64_t)l;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      int8_t l = (int8_t)left;
      l >>= right;
      left = (uint8_t)l;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      int8_t l = (int8_t)left;
      l >>= (uint8_t)right;
      left = (uint8_t)l;
      }
    };

  struct ShrOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left >>= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left >>= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left >>= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left >>= (uint8_t)right;
      }
    };

  struct SubOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left -= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left -= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left -= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left -= (uint8_t)right;
      }
    };

  struct XorOper
    {
    static void apply(uint64_t& left, uint64_t right)
      {
      left ^= right;
      }
    static void apply(uint64_t& left, uint8_t right)
      {
      left ^= (uint64_t)right;
      }
    static void apply(uint8_t& left, uint8_t right)
      {
      left ^= right;
      }
    static void apply(uint8_t& left, uint64_t right)
      {
      left ^= (uint8_t)right;
      }
    };

  struct AddsdOper
    {
    static void apply(double& left, double right)
      {
      left += right;
      }
    };

  struct DivsdOper
    {
    static void apply(double& left, double right)
      {
      left /= right;
      }
    };

  struct MulsdOper
    {
    static void apply(double& left, double right)
      {
      left *= right;
      }
    };

  struct SubsdOper
    {
    static void apply(double& left, double right)
      {
      left -= right;
      }
    };

  template <class TOper>
  inline void execute_operation(asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem,
    registers& regs)
    {
    uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
    if (oprnd1)
      {
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      if (oprnd2)
        TOper::apply(*oprnd1, *oprnd2);
      else if (operand2 == asmcode::NUMBER || operand2 == asmcode::LABELADDRESS)
        TOper::apply(*oprnd1, operand2_mem);
      else
        {
        uint8_t* oprnd2_8 = get_address_8bit(operand2, operand2_mem, regs);
        if (oprnd2_8)
          TOper::apply(*oprnd1, *oprnd2_8);
        }
      }
    else
      {
      uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
      if (oprnd1_8)
        {
        uint8_t* oprnd2_8 = get_address_8bit(operand2, operand2_mem, regs);
        if (oprnd2_8)
          TOper::apply(*oprnd1_8, *oprnd2_8);
        else if (operand2 == asmcode::NUMBER)
          TOper::apply(*oprnd1_8, operand2_mem);
        }
      }
    }

  template <class TOper>
  inline void execute_double_operation(asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem,
    registers& regs)
    {
    uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
    if (oprnd1)
      {
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      if (oprnd2)
        TOper::apply(*reinterpret_cast<double*>(oprnd1), *reinterpret_cast<double*>(oprnd2));
      }
    }

  template <class TOper>
  inline uint64_t execute_operation_const(asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem,
    registers& regs)
    {
    uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
    if (oprnd1)
      {
      uint64_t left = *oprnd1;
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      if (oprnd2)
        {
        TOper::apply(left, *oprnd2);
        return left;
        }
      else if (operand2 == asmcode::NUMBER || operand2 == asmcode::LABELADDRESS)
        {
        TOper::apply(left, operand2_mem);
        return left;
        }
      else
        {
        uint8_t* oprnd2_8 = get_address_8bit(operand2, operand2_mem, regs);
        if (oprnd2_8)
          {
          TOper::apply(left, *oprnd2_8);
          return left;
          }
        }
      }
    else
      {
      uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
      uint8_t left = *oprnd1_8;
      if (oprnd1_8)
        {
        uint8_t* oprnd2_8 = get_address_8bit(operand2, operand2_mem, regs);
        if (oprnd2_8)
          {
          TOper::apply(left, *oprnd2_8);
          return (uint64_t)left;
          }
        else if (operand2 == asmcode::NUMBER)
          {
          TOper::apply(left, operand2_mem);
          return (uint64_t)left;
          }
        }
      }
    throw std::logic_error("Invalid bytecode");
    }

  inline void get_values(int64_t& left_signed, int64_t& right_signed, uint64_t& left_unsigned, uint64_t right_unsigned, asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem,
    registers& regs)
    {
    uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
    if (oprnd1)
      {
      left_unsigned = *oprnd1;
      left_signed = (int64_t)left_unsigned;
      }
    else
      {
      uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
      left_unsigned = *oprnd1_8;
      left_signed = (int8_t)(*oprnd1_8);
      }
    uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
    if (oprnd2)
      {
      right_unsigned = *oprnd2;
      right_signed = (int64_t)right_unsigned;
      }
    else if (operand2 == asmcode::NUMBER || operand2 == asmcode::LABELADDRESS)
      {
      right_unsigned = operand2_mem;
      right_signed = (int64_t)right_unsigned;
      }
    else
      {
      uint8_t* oprnd2_8 = get_address_8bit(operand2, operand2_mem, regs);
      right_unsigned = *oprnd2_8;
      right_signed = (int8_t)(*oprnd2_8);
      }
    }

  inline void compare_operation(asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem,
    registers& regs)
    {
    regs.eflags = 0;
    int64_t left_signed = 0;
    int64_t right_signed = 0;
    uint64_t left_unsigned = 0;
    uint64_t right_unsigned = 0;
    get_values(left_signed, right_signed, left_unsigned, right_unsigned, operand1, operand2, operand1_mem, operand2_mem, regs);
    if (left_signed == right_signed)
      regs.eflags |= zero_flag;
    if (left_unsigned < right_unsigned)
      regs.eflags |= carry_flag;
    int64_t temp = left_signed - right_signed;
    if ((temp < left_signed) != (right_signed > 0))
      regs.eflags |= overflow_flag;
    if (temp < 0)
      regs.eflags |= sign_flag;
    }

  void print(asmcode::operation op, asmcode::operand operand1,
    asmcode::operand operand2,
    uint64_t operand1_mem,
    uint64_t operand2_mem)
    {
    asmcode::instruction i;
    i.oper = op;
    i.operand1 = operand1;
    i.operand2 = operand2;
    i.operand1_mem = operand1_mem;
    i.operand2_mem = operand2_mem;
    i.stream(std::cout);
    }

  } // namespace
void run_bytecode(const uint8_t* bytecode, uint64_t size, registers& regs)
  {
  (void*)size;

  regs.rsp -= 8;
  *((uint64_t*)regs.rsp) = 0xffffffffffffffff; // this address means the function call representing this bytecode

  const uint8_t* bytecode_ptr = bytecode;

  for (;;)
    {
    asmcode::operation op;
    asmcode::operand operand1;
    asmcode::operand operand2;
    uint64_t operand1_mem;
    uint64_t operand2_mem;
    uint64_t sz = disassemble_bytecode(op, operand1, operand2, operand1_mem, operand2_mem, bytecode_ptr);

    //print(op, operand1, operand2, operand1_mem, operand2_mem);

    switch (op)
      {
      case asmcode::ADD:
      {
      execute_operation<AddOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::ADDSD:
      {
      execute_double_operation<AddsdOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::AND:
      {
      execute_operation<AndOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::CALL:
      {
      if (operand1 == asmcode::NUMBER) // local call
        {
        regs.rsp -= 8;
        *((uint64_t*)regs.rsp) = (uint64_t)(bytecode_ptr + sz); // save address right after call on stack
        uint32_t local_offset = (uint32_t)operand1_mem;
        bytecode_ptr += (int32_t)local_offset;
        sz = 0;
        }
      else // external call
        {
        uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
        regs.rsp -= 8;
        *((uint64_t*)regs.rsp) = (uint64_t)(bytecode_ptr + sz); // save address right after call on stack
        bytecode_ptr = (const uint8_t*)(*oprnd1);
        sz = 0;
        //throw std::logic_error("external call not implemented");
        }
      break;
      }
      case asmcode::CMP:
      {
      compare_operation(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::CMPEQPD:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      double v1 = *reinterpret_cast<double*>(oprnd1);
      double v2 = *reinterpret_cast<double*>(oprnd2);
      *oprnd1 = (v1 == v2) ? 0xffffffffffffffff : 0;
      break;
      }
      case asmcode::CMPLTPD:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      double v1 = *reinterpret_cast<double*>(oprnd1);
      double v2 = *reinterpret_cast<double*>(oprnd2);
      *oprnd1 = (v1 < v2) ? 0xffffffffffffffff : 0;
      break;
      }
      case asmcode::CMPLEPD:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      double v1 = *reinterpret_cast<double*>(oprnd1);
      double v2 = *reinterpret_cast<double*>(oprnd2);
      *oprnd1 = (v1 <= v2) ? 0xffffffffffffffff : 0;
      break;
      }
      case asmcode::CVTSI2SD:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      double v = (double)((int64_t)*oprnd2);
      *reinterpret_cast<double*>(oprnd1) = v;
      break;
      }
      case asmcode::CVTTSD2SI:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      double v = *reinterpret_cast<double*>(oprnd2);
      *oprnd1 = (int64_t)v;
      break;
      }
      case asmcode::DEC:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      if (oprnd1)
        {
        *oprnd1 -= 1;
        if (*oprnd1)
          regs.eflags &= ~zero_flag;
        else
          regs.eflags |= zero_flag;
        }
      else
        {
        uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
        if (oprnd1_8)
          {
          *oprnd1_8 -= 1;
          if (*oprnd1_8)
            regs.eflags &= ~zero_flag;
          else
            regs.eflags |= zero_flag;
          }
        }
      break;
      }
      case asmcode::DIVSD:
      {
      execute_double_operation<DivsdOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::IMUL:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      if (oprnd1)
        {
        int64_t rax = (int64_t)regs.rax;
        rax *= (int64_t)(*oprnd1);
        regs.rax = rax;
        }
      else
        {
        uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
        int8_t rax = ((int64_t)regs.rax) & 255;
        rax *= (int8_t)(*oprnd1_8);
        regs.rax &= 0xffffffffffffff00;
        regs.rax |= rax;
        }
      break;
      }
      case asmcode::INC:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      if (oprnd1)
        {
        *oprnd1 += 1;
        if (*oprnd1)
          regs.eflags &= ~zero_flag;
        else
          regs.eflags |= zero_flag;
        }
      else
        {
        uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
        if (oprnd1_8)
          {
          *oprnd1_8 += 1;
          if (*oprnd1_8)
            regs.eflags &= ~zero_flag;
          else
            regs.eflags |= zero_flag;
          }
        }
      break;
      }
      case asmcode::JA:
      case asmcode::JAS:
      {
      if (((regs.eflags & zero_flag) | (regs.eflags & carry_flag)) == 0)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("ja(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JB:
      case asmcode::JBS:
      {
      if (regs.eflags & carry_flag)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jb(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JE:
      case asmcode::JES:
      {
      if (regs.eflags & zero_flag)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("je(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JG:
      case asmcode::JGS:
      {
      if ((((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)) | (regs.eflags & zero_flag)) == 0)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jg(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JGE:
      case asmcode::JGES:
      {
      if (((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)) == 0)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jge(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JL:
      case asmcode::JLS:
      {
      if (((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)))
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jl(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JLE:
      case asmcode::JLES:
      {
      if ((((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)) | (regs.eflags & zero_flag)))
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jle(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JNE:
      case asmcode::JNES:
      {
      if ((regs.eflags & zero_flag) == 0)
        {
        if (operand1 == asmcode::NUMBER)
          {
          int32_t local_offset = (int32_t)operand1_mem;
          bytecode_ptr += local_offset;
          sz = 0;
          }
        else
          {
          throw std::logic_error("jne(s) not implemented");
          }
        }
      break;
      }
      case asmcode::JMPS:
      {
      if (operand1 == asmcode::NUMBER)
        {
        int32_t local_offset = (int32_t)operand1_mem;
        bytecode_ptr += local_offset;
        sz = 0;
        }
      else
        {
        throw std::logic_error("jmps not implemented");
        }
      break;
      }
      case asmcode::JMP:
      {
      if (operand1 == asmcode::NUMBER)
        {
        int32_t local_offset = (int32_t)operand1_mem;
        bytecode_ptr += local_offset;
        sz = 0;
        }
      else
        {
        uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
        bytecode_ptr = (const uint8_t*)(*oprnd1);
        sz = 0;
        }
      break;
      }
      case asmcode::MOVMSKPD:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint64_t* oprnd2 = get_address_64bit(operand2, operand2_mem, regs);
      *oprnd1 = (*oprnd2) ? 1 : 0;
      break;
      }
      case asmcode::MOVSD:
      case asmcode::MOVQ:
      case asmcode::MOV:
      {
      execute_operation<MovOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::MOVZX:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      uint8_t* oprnd2 = get_address_8bit(operand2, operand2_mem, regs);
      *oprnd1 = *oprnd2;
      break;
      }
      case asmcode::MUL:
      {
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      if (oprnd1)
        {
        regs.rax *= (uint64_t)(*oprnd1);
        }
      else
        {
        uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
        uint8_t rax = regs.rax & 255;
        rax *= (uint8_t)(*oprnd1_8);
        regs.rax &= 0xffffffffffffff00;
        regs.rax |= rax;
        }
      break;
      }
      case asmcode::MULSD:
      {
      execute_double_operation<MulsdOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::NOP: break;
      case asmcode::OR:
      {
      execute_operation<OrOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::POP:
      {
      uint64_t address = *((uint64_t*)regs.rsp);
      regs.rsp += 8;
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      *oprnd1 = address;
      break;
      }
      case asmcode::PUSH:
      {
      regs.rsp -= 8;
      uint64_t* oprnd1 = get_address_64bit(operand1, operand1_mem, regs);
      if (oprnd1)
        *((uint64_t*)regs.rsp) = *oprnd1;
      else if (operand1 == asmcode::NUMBER || operand1 == asmcode::LABELADDRESS)
        *((uint64_t*)regs.rsp) = operand1_mem;
      else
        {
        uint8_t* oprnd1_8 = get_address_8bit(operand1, operand1_mem, regs);
        *((uint64_t*)regs.rsp) = (int8_t)(*oprnd1_8);
        }
      break;
      }
      case asmcode::RET:
      {
      uint64_t address = *((uint64_t*)regs.rsp);
      regs.rsp += 8; // to check, might need to pop more
      if (address == 0xffffffffffffffff) // we're at the end of this bytecode function call
        return;
      bytecode_ptr = (const uint8_t*)address;
      sz = 0;
      break;
      }
      case asmcode::SAL:
      {
      execute_operation<ShlOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::SAR:
      {
      execute_operation<SarOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::SETE:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if (regs.eflags & zero_flag)
        *oprnd1 = 1;
      else
        *oprnd1 = 0;
      break;
      }
      case asmcode::SETNE:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if (regs.eflags & zero_flag)
        *oprnd1 = 0;
      else
        *oprnd1 = 1;
      break;
      }
      case asmcode::SETL:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if (((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)))
        *oprnd1 = 1;
      else
        *oprnd1 = 0;
      break;
      }
      case asmcode::SETLE:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if ((((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)) | (regs.eflags & zero_flag)) == 0)
        *oprnd1 = 0;
      else
        *oprnd1 = 1;
      break;
      }
      case asmcode::SETG:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if ((((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)) | (regs.eflags & zero_flag)) == 0)
        *oprnd1 = 1;
      else
        *oprnd1 = 0;
      break;
      }
      case asmcode::SETGE:
      {
      uint8_t* oprnd1 = get_address_8bit(operand1, operand1_mem, regs);
      if (((regs.eflags & sign_flag) ^ (regs.eflags & overflow_flag)))
        *oprnd1 = 0;
      else
        *oprnd1 = 1;
      break;
      }
      case asmcode::SHL:
      {
      execute_operation<ShlOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::SHR:
      {
      execute_operation<ShrOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::SUB:
      {
      execute_operation<SubOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::SUBSD:
      {
      execute_double_operation<SubsdOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      case asmcode::TEST:
      {
      uint64_t tmp = execute_operation_const<AndOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      if (tmp)
        {
        regs.eflags &= ~zero_flag;
        if ((int64_t)tmp < 0)
          regs.eflags |= sign_flag;
        else
          regs.eflags &= ~sign_flag;
        }
      else
        {
        regs.eflags |= zero_flag;
        regs.eflags &= ~sign_flag;
        }
      break;
      }
      case asmcode::XOR:
      {
      execute_operation<XorOper>(operand1, operand2, operand1_mem, operand2_mem, regs);
      break;
      }
      default:
      {
      std::stringstream str;
      str << asmcode::operation_to_string(op) << " is not implemented yet!";
      throw std::logic_error(str.str());
      }
      }
    bytecode_ptr += sz;
    }
  }

ASM_END