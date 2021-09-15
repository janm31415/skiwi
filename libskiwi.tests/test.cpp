#include "test_assert.h"

#include "compile_tests.h"
#include "compile_vm_tests.h"
#include "conversion_tests.h"
#include "format_tests.h"
#include "parse_tests.h"
#include "tokenize_tests.h"

#include <ctime>

int main(int /*argc*/, const char* /*argv*/[])
  {
  InitTestEngine();

  auto tic = std::clock();
  run_all_format_tests();
  run_all_parse_tests();
  run_all_tokenize_tests();
  run_all_conversion_tests();
  run_all_compile_tests();
  run_all_compile_vm_tests();
  auto toc = std::clock();

  if (!testing_fails) 
    {
    TEST_OUTPUT_LINE("Succes: %d tests passed.", testing_success);
    }
  else 
    {
    TEST_OUTPUT_LINE("FAILURE: %d out of %d tests failed (%d failures).", testing_fails, testing_success+testing_fails, testing_fails);
    }
  TEST_OUTPUT_LINE("Test time: %f seconds.", (double)(toc - tic)/(double)CLOCKS_PER_SEC);
  return CloseTestEngine(true);
  }
