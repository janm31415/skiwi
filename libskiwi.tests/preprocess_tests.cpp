#include "preprocess_tests.h"
#include "test_assert.h"
#include "libskiwi/preprocess.h"
#include "libskiwi/dump.h"
#include "libskiwi/cinput_data.h"
#include <string>
#include <sstream>
#include <iostream>

SKIWI_BEGIN

namespace
  {
  std::string to_string(Program& prog)
    {
    std::stringstream str;
    dump(str, prog);
    return str.str();
    }
    
  void preprocess_let_statements()
    {
    std::string script = "(let ([x (+ 1 2)]) (let([y(+ 3 4)])(+ x y)))";
    auto tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    auto prog = make_program(tokens);
    context ctxt = create_context(1024, 1024, 1024, 1024);
    environment_map env = std::make_shared<environment<environment_entry>>(nullptr);
    repl_data rd;
    macro_data md;
    primitive_map pm;
    compiler_options ops;
    cinput_data cinp;
    preprocess(env, rd, md, ctxt, cinp, prog, pm, ops);
    std::string preprocessed_script = R"(( let ( [ x_0 ( if ( ##eq? + ###+ ) 3 ( + 1 2 ) ) ] ) ( let ( [ y_1 ( if ( ##eq? + ###+ ) 7 ( + 3 4 ) ) ] ) ( let ( [ #%k0 ( if ( ##eq? + ###+ ) ( if ( if ( ##fixnum? x_0 ) ( ##fixnum? y_1 ) #f ) ( ##fx+ x_0 y_1 ) ( if ( if ( ##flonum? x_0 ) ( ##flonum? y_1 ) #f ) ( ##fl+ x_0 y_1 ) ( + x_0 y_1 ) ) ) ( + x_0 y_1 ) ) ] ) ( halt #%k0 ) ) ) ) )";
    TEST_ASSERT(to_string(prog)==preprocessed_script);
    
    script = "(if (char? #\\a) 13 14)";
    tokens = tokenize(script);
    std::reverse(tokens.begin(), tokens.end());
    prog = make_program(tokens);
    ops.primitives_inlined = false;
    preprocess(env, rd, md, ctxt, cinp, prog, pm, ops);
    TEST_ASSERT(to_string(prog) == "( if ( char? #\\97 ) ( halt 13 ) ( halt 14 ) ) ");
    }
  
  }

SKIWI_END

void run_all_preprocess_tests()
  {
  using namespace SKIWI;
  preprocess_let_statements();
  }
