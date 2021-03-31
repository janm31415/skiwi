///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "AssemblerTests.h"
#include <stdint.h>
#include "asm/assembler.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>

#include "test_assert.h"

ASM_BEGIN


namespace
  {
  void assembler_add()
    {
    asmcode code;
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);

    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }


  void assembler_call()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }


  void assembler_call_2()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label_2");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }


  void assembler_jmp()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }




  void assembler_call_aligned()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }


  void assembler_call_2_aligned()
    {
    asmcode code;
    code.add(asmcode::CALL, "L_label");
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label_2");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
    code.add(asmcode::CALL, "L_label_2");
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }


  void assembler_jmp_aligned()
    {
    asmcode code;
    code.add(asmcode::JMP, "L_label");
    code.add(asmcode::LABEL_ALIGNED, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }
  namespace
    {

    double get_value()
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

    typedef uint64_t(*fun_ptr)();
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      double d = *reinterpret_cast<double*>(&res);
      TEST_EQ(5.678, d);
      free_assembled_function((void*)f, size);
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

    typedef uint64_t(*fun_ptr)();

    std::map<std::string, uint64_t> external_functions;
    external_functions["get_value"] = get_value_address;
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code, external_functions);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f();
      double d = *reinterpret_cast<double*>(&res);
      TEST_EQ(5.678, d);
      free_assembled_function((void*)f, size);
      }
    }

  void assembler_move_label_to_rax_and_call_rax()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::LABELADDRESS, "L_label");
    code.add(asmcode::CALL, asmcode::RAX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }

  void assembler_move_label_to_rax_and_call_rax_aligned()
    {
    asmcode code;
    code.add(asmcode::MOV, asmcode::RAX, asmcode::LABELADDRESS, "L_label");
    code.add(asmcode::CALL, asmcode::RAX);
    code.add(asmcode::RET);
    code.add(asmcode::LABEL_ALIGNED, "L_label");
#ifdef _WIN32
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RCX);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDX);
#else
    code.add(asmcode::MOV, asmcode::RAX, asmcode::RSI);
    code.add(asmcode::ADD, asmcode::RAX, asmcode::RDI);
#endif
    code.add(asmcode::RET);

    typedef uint64_t(*fun_ptr)(uint64_t, ...);
    uint64_t size;
    fun_ptr f = (fun_ptr)assemble(size, code);

    TEST_ASSERT(f != NULL);

    if (f)
      {
      uint64_t res = f(3, 7);
      TEST_EQ(res, uint64_t(10));
      res = f(100, 211);
      TEST_EQ(res, uint64_t(311));
      free_assembled_function((void*)f, size);
      }

    }

  }

ASM_END


void run_all_assembler_tests()
  {
  using namespace ASM;

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
  }