#include "include_handler.h"

#include <cassert>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "visitor.h"
#include "filename_setter.h"
#include "compile_error.h"

SKIWI_BEGIN

namespace
  {
  struct include_handler_visitor : public base_visitor<include_handler_visitor>
    {
    void _revisit_expr(Expression& e)
      {
      visitor<Expression, include_handler_visitor>::visit(e, this);
      }

    Program _include(Expression& e)
      {
      assert(std::holds_alternative<PrimitiveCall>(e));
      PrimitiveCall& p = std::get<PrimitiveCall>(e);
      assert(p.primitive_name == "include");
      if (p.arguments.size() != 1)
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments, "include");
      if (!std::holds_alternative<Literal>(p.arguments.front()))
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument, "I expect a string as argument");
      Literal& lit = std::get<Literal>(p.arguments.front());
      if (!std::holds_alternative<String>(lit))
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument, "I expect a string as argument");
      std::string filename = std::get<String>(lit).value;
      std::ifstream rfile;
      rfile.open(filename);
      if (!rfile.is_open())
        {
        std::stringstream ss;
        ss << "Invalid file: " << filename;
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument, ss.str());
        }
      std::stringstream str;
      std::string line;
      while (std::getline(rfile, line)) 
        {
        str << line << std::endl;
        }
      rfile.close();
      auto tokens = tokenize(str.str());
      std::reverse(tokens.begin(), tokens.end());
      Program prog;
      try
        {
        prog = make_program(tokens);
        }
      catch (std::logic_error e)
        {        
        std::stringstream error_str;
        error_str << "in file " << filename << ": " << e.what();
        throw_error(p.line_nr, p.column_nr, p.filename, bad_syntax, error_str.str());
        }
      set_filename(prog, filename);
      return prog;
      }

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<PrimitiveCall>(e))
        {
        PrimitiveCall& p = std::get<PrimitiveCall>(e);
        if (p.primitive_name == "include")
          {
          auto prog = _include(e);
          Begin new_begin;
          new_begin.arguments.swap(prog.expressions);
          e = new_begin;
          _revisit_expr(e);
          }
        }
      return true;
      }

    virtual bool _previsit(Program& p)
      {
      bool found_include = true;
      while (found_include)
        {
        found_include = false;
        for (auto it = p.expressions.begin(); it != p.expressions.end(); ++it)
          {
          Expression& e = *it;
          if (std::holds_alternative<PrimitiveCall>(e))
            {
            PrimitiveCall& prim = std::get<PrimitiveCall>(e);
            if (prim.primitive_name == "include")
              {
              auto prog = _include(e);
              found_include = true;
              it = p.expressions.erase(it);
              p.expressions.insert(it, prog.expressions.begin(), prog.expressions.end());
              }
            }
          if (found_include)
            break;
          }
        }
      return true;
      }

    };
  }

void handle_include_command(Program& prog)
  {
  assert(!prog.include_handled);
  include_handler_visitor lhv;
  visitor<Program, include_handler_visitor>::visit(prog, &lhv);
  prog.include_handled = true;
  }

SKIWI_END
