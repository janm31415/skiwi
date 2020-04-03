///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "tokenize_tests.h"
#include "test_assert.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include <libskiwi/tokenize.h>

SKIWI_BEGIN

namespace
  {

  void tokenize_symbol()
    {
    auto tokens = tokenize("#f");
    TEST_EQ(int(1), int(tokens.size()));
    TEST_EQ(token::T_SYMBOL, tokens.front().type);
    TEST_EQ(std::string("#f"), tokens.front().value);

    tokens = tokenize("#\\nul");
    TEST_EQ(int(1), int(tokens.size()));
    TEST_EQ(token::T_SYMBOL, tokens.front().type);
    TEST_EQ(std::string("#\\nul"), tokens.front().value);
    }

  void tokenize_list()
    {
    auto tokens = tokenize("(list a b)");
    TEST_EQ(int(5), int(tokens.size()));
    TEST_EQ(token::T_LEFT_ROUND_BRACKET, tokens[0].type);
    TEST_EQ(token::T_ID, tokens[1].type);
    TEST_EQ(token::T_ID, tokens[2].type);
    TEST_EQ(token::T_ID, tokens[3].type);
    TEST_EQ(token::T_RIGHT_ROUND_BRACKET, tokens[4].type);
    }

  void tokenize_string()
    {
    auto tokens = tokenize("(print\"spaghetti () \")");
    TEST_EQ(int(4), int(tokens.size()));
    TEST_EQ(token::T_LEFT_ROUND_BRACKET, tokens[0].type);
    TEST_EQ(token::T_ID, tokens[1].type);
    TEST_EQ(token::T_STRING, tokens[2].type);
    TEST_EQ(std::string("\"spaghetti () \""), tokens[2].value);
    TEST_EQ(token::T_RIGHT_ROUND_BRACKET, tokens[3].type);
    }

  void tokenize_fixnum_real()
    {
    auto tokens = tokenize("(+ 1 2.0)");
    TEST_EQ(int(5), int(tokens.size()));
    TEST_EQ(token::T_LEFT_ROUND_BRACKET, tokens[0].type);
    TEST_EQ(token::T_ID, tokens[1].type);
    TEST_EQ(token::T_FIXNUM, tokens[2].type);
    TEST_EQ(token::T_FLONUM, tokens[3].type);
    TEST_EQ(token::T_RIGHT_ROUND_BRACKET, tokens[4].type);
    }
  }

SKIWI_END

void run_all_tokenize_tests()
  {
  using namespace SKIWI;
  void tokenize_symbol();
  void tokenize_list();
  void tokenize_string();
  void tokenize_fixnum_real();
  }