///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "AssemblerTests.h"
#include <stdint.h>
#include <asm/assembler.h>
#include <windows.h>
#include <iostream>

#include "test_assert.h"

SKIWI_BEGIN

namespace
  {
  void assembler_add()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }


  void assembler_call()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }


  void assembler_call_2()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label_2");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }


  void assembler_jmp()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }




  void assembler_call_aligned()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }


  void assembler_call_2_aligned()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label_2");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }


  void assembler_jmp_aligned()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }
  namespace
    {

    double _cdecl get_value()
      {
      return 5.678;
      }

    }


  void assembler_call_external_via_register()
    {
    uint64_t get_value_address = (uint64_t)(&get_value);

    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, get_value_address);
    code.add(asmcode::CALL, asmcode::RAX);
    code.add(asmcode::MOVQ, asmcode::RAX, asmcode::XMM0);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)();

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      double d = *reinterpret_cast<double*>(&res);
      TEST_EQ(5.678, d);
      free_assembled_function(f);
      }
    }


  void assembler_call_external()
    {
    asmcode code;
    code.add(asmcode::EXTERN, "get_value");
    code.add(asmcode::CALL, "get_value");
    code.add(asmcode::MOVQ, asmcode::RAX, asmcode::XMM0);
    code.add(asmcode::RET);

    uint64_t get_value_address = (uint64_t)(&get_value);

    typedef uint64_t(__cdecl *fun_ptr)();

    std::map<std::string, uint64_t> external_functions;
    external_functions["get_value"] = get_value_address;

    fun_ptr f = (fun_ptr)assemble(code, external_functions);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      double d = *reinterpret_cast<double*>(&res);
      TEST_EQ(5.678, d);
      free_assembled_function(f);
      }
    }

  void assembler_move_label_to_rax_and_call_rax()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::LABELADDRESS, "L_label");
    code.add(asmcode::CALL, asmcode::RAX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }

  void assembler_move_label_to_rax_and_call_rax_aligned()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::LABELADDRESS, "L_label");
    code.add(asmcode::CALL, asmcode::RAX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
    code.add(asmcode::RET);

    typedef uint64_t(__cdecl *fun_ptr)(uint64_t, ...);

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function(f);
      }

    }

  void dq_memory()
    {
    asmcode code;
    code.add(asmcode::GLOBAL, "entry");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
    code.add(asmcode::MOV, asmcode::MOFFS64, asmcode::RAX, "my_var");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 4);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MOFFS64, "my_var");
    code.add(asmcode::RET);
    code.add(asmcode::DATA);
    code.add(asmcode::DQ, asmcode::NUMBER, 100, "my_var");

    typedef uint64_t(__cdecl *fun_ptr)();

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      TEST_EQ(res, uint64_t(2));
      free_assembled_function(f);
      }

    }

  void dq_memory_2()
    {
    asmcode code;
    code.add(asmcode::GLOBAL, "entry");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MOFFS64, "my_var");
    code.add(asmcode::RET);
    code.add(asmcode::DATA);
    code.add(asmcode::DQ, asmcode::NUMBER, 100, "my_var");

    typedef uint64_t(__cdecl *fun_ptr)();

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      TEST_EQ(res, uint64_t(100));
      free_assembled_function(f);
      }

    }

  void dq_memory_3()
    {
    asmcode code;
    code.add(asmcode::GLOBAL, "entry");
    code.add(asmcode::XOR, asmcode::RCX, asmcode::RCX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MOFFS64, "var1");
    code.add(asmcode::ADD, asmcode::RCX, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MOFFS64, "var2");
    code.add(asmcode::ADD, asmcode::RCX, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MOFFS64, "var3");
    code.add(asmcode::ADD, asmcode::RCX, asmcode::RAX);
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::RET);
    code.add(asmcode::DATA);
    code.add(asmcode::DQ, asmcode::NUMBER, 11, "var1");
    code.add(asmcode::DQ, asmcode::NUMBER, 12, "var2");
    code.add(asmcode::DQ, asmcode::NUMBER, 13, "var3");
    typedef uint64_t(__cdecl *fun_ptr)();

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      TEST_EQ(res, uint64_t(36));
      free_assembled_function(f);
      }

    }

  void dq_memory_4()
    {
    asmcode code;
    code.add(asmcode::GLOBAL, "entry");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::NUMBER, 2);
    code.add(asmcode::MOV, asmcode::RCX, asmcode::VARIABLE, "my_var");
    code.add(asmcode::MOV, asmcode::RAX, asmcode::MEM_RCX);
    code.add(asmcode::RET);
    code.add(asmcode::DATA);
    code.add(asmcode::DQ, asmcode::NUMBER, 100, "my_var");

    typedef uint64_t(__cdecl *fun_ptr)();

    fun_ptr f = (fun_ptr)assemble(code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      TEST_EQ(res, uint64_t(100));
      free_assembled_function(f);
      }

    }
  }

SKIWI_END


void run_all_assembler_tests()
  {
  using namespace SKIWI;
  assembler_add();
  assembler_call();
  assembler_call_2();
  assembler_jmp();
  assembler_call_aligned();
  assembler_call_2_aligned();
  assembler_jmp_aligned();
  assembler_call_external_via_register();
  assembler_call_external();
  assembler_move_label_to_rax_and_call_rax();
  assembler_move_label_to_rax_and_call_rax_aligned();
  dq_memory();
  dq_memory_2();
  dq_memory_3();
  dq_memory_4();
  }