#include "macro_expander.h"
#include "visitor.h"
#include "compile_error.h"
#include "debug_find.h"
#include <cassert>
#include <map>
#include <iostream>
#include <sstream>

#include <asm/assembler.h>

#include "parse.h"
#include "context.h"

#include "runtime.h"
#include "dump.h"

SKIWI_BEGIN

namespace
  {
  Variable _make_var(const std::string& name)
    {
    Variable v;
    v.name = name;
    return v;
    }

  bool is_name(const Expression& expr)
    {
    if (std::holds_alternative<PrimitiveCall>(expr))
      return std::get<PrimitiveCall>(expr).as_object;
    return std::holds_alternative<Variable>(expr);
    }

  std::string get_name(const Expression& expr)
    {
    if (std::holds_alternative<Variable>(expr))
      return std::get<Variable>(expr).name;
    if (std::holds_alternative<PrimitiveCall>(expr))
      return std::get<PrimitiveCall>(expr).primitive_name;
    return "";
    }


  struct macro_finder_visitor : public base_visitor<macro_finder_visitor>
    {
    std::vector<Expression> macros;
    std::vector<Expression> lisp_defmacros;

    void find_macros(std::vector<Expression>& exprs)
      {
      std::vector<size_t> exprs_to_delete;
      for (size_t i = 0; i < exprs.size(); ++i)
        {
        auto& expr = exprs[i];
        if (std::holds_alternative<PrimitiveCall>(expr))
          {
          PrimitiveCall& p = std::get<PrimitiveCall>(expr);
          if (p.primitive_name == "define-macro")
            {
            macros.push_back(p);
            exprs_to_delete.push_back(i);
            }
          if (p.primitive_name == "defmacro")
            {
            lisp_defmacros.push_back(p);
            exprs_to_delete.push_back(i);
            }
          }
        }
      for (auto rit = exprs_to_delete.rbegin(); rit != exprs_to_delete.rend(); ++rit)
        {
        exprs.erase(exprs.begin() + *rit);
        }
      }

    virtual void _postvisit(Program& prog)
      {
      
      if (prog.expressions.size() == 1 && std::holds_alternative<Begin>(prog.expressions.front()))
        {
        Begin& b = std::get<Begin>(prog.expressions.front());
        find_macros(b.arguments);
        }
      else
        find_macros(prog.expressions);
        
      //find_macros(prog.expressions);
      }

    virtual void _postvisit(Begin&)
      {
      //find_macros(b.arguments);
      }
    };

  cell expression_to_cell(Expression& expr)
    {
    std::stringstream str;
    dump(str, expr, false);
    auto tokens = tokenize(str.str());
    std::reverse(tokens.begin(), tokens.end());
    return read_from(tokens, true);
    }

  struct macro_expander_visitor : public base_visitor<macro_expander_visitor>
    {
    macro_data* p_md;
    context* p_ctxt;
    const primitive_map* p_pm;
    compiler_options ops;
    environment_map* p_env;
    repl_data* p_rd;
    bool macros_expanded;

    macro_expander_visitor() : macros_expanded(false) {}

    virtual bool _previsit(Expression& e)
      {
      if (std::holds_alternative<FunCall>(e))
        {
        auto& f = std::get<FunCall>(e);
        if (std::holds_alternative<Variable>(f.fun.front()))
          {
          auto it = p_md->m.find(std::get<Variable>(f.fun.front()).name);
          if (it != p_md->m.end())
            {
            if (!it->second.variable_arity)
              {
              if (it->second.variables.size() != f.arguments.size())
                throw_error(f.line_nr, f.column_nr, f.filename, macro_invalid_pattern, it->first);
              }
            else
              {
              if (it->second.variables.size() > f.arguments.size()+1)
                throw_error(f.line_nr, f.column_nr, f.filename, macro_invalid_pattern, it->first);
              }
            macros_expanded = true;
            for (auto& arg : f.arguments)
              {
              Quote q;
              q.arg = expression_to_cell(arg);
              arg = q;
              }
            Program pr;
            pr.expressions.push_back(f);
            std::map<std::string, external_function> externals;
            asmcode code;
            try
              {
              macro_data md; // all macros are in the environment
              compile(*p_env, *p_rd, md, *p_ctxt, code, pr, *p_pm, externals, ops);

              typedef uint64_t(__cdecl *compiled_fun_ptr)(void*);

              first_pass_data d;
              uint64_t fie_size;
              compiled_fun_ptr fie = (compiled_fun_ptr)assemble(fie_size, d, code);

              if (fie)
                {
                auto res = fie(p_ctxt);
                std::stringstream str;
                scheme_runtime(res, str, *p_env, p_ctxt);
                std::string script = str.str();
                //printf("%s\n", script.c_str());
                auto tokens = tokenize(script);
                std::reverse(tokens.begin(), tokens.end());
                auto result = make_program(tokens);

                if (result.expressions.empty())
                  e = Nop();
                else if (result.expressions.size() == 1)
                  e = result.expressions.front();
                else
                  {
                  Begin b;
                  b.arguments.swap(result.expressions);
                  e = b;
                  }

                free_assembled_function(fie, fie_size);
                }
              else
                e = Nop();
              }
            catch (std::logic_error er)
              {
              std::cout << er.what() << "\n";
              throw er;
              }
            catch (std::runtime_error er)
              {
              std::cout << er.what() << "\n";
              throw er;
              }

            return false;
            }
          }
        }
      return true;
      }

    };

  Program get_macros(Program& prog, macro_data& md)
    {
    Program out;

    macro_finder_visitor mfv;
    visitor<Program, macro_finder_visitor>::visit(prog, &mfv);    

    for (auto& mac : mfv.macros)
      {
      assert(std::holds_alternative<PrimitiveCall>(mac));
      auto& p = std::get<PrimitiveCall>(mac);
      if (p.arguments.size() < 2)
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments);
      assert(p.arguments.size() >= 2);
      if (!std::holds_alternative<FunCall>(p.arguments.front()))
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
      FunCall f = std::get<FunCall>(p.arguments.front());
      if (!std::holds_alternative<Variable>(f.fun.front()))
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
      bool variable_arity = false;
      std::vector<std::string> variable_names;
      std::string macro_name = std::get<Variable>(f.fun.front()).name;
      if (f.arguments.size() >= 2 && std::holds_alternative<Literal>(f.arguments[f.arguments.size() - 2]))
        {
        Literal& lit = std::get<Literal>(f.arguments[f.arguments.size() - 2]);
        if (std::holds_alternative<Flonum>(lit))
          {
          Flonum& fl = std::get<Flonum>(lit);
          if (fl.value != 0.0)
            throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
          for (size_t j = 0; j < f.arguments.size(); ++j)
            {
            if (j == (f.arguments.size() - 2))
              continue;
            if (!is_name(f.arguments[j]))
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
            variable_names.push_back(get_name(f.arguments[j]));
            }
          variable_arity = true;
          }
        else
          throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
        }
      else
        {
        for (const auto& arg : f.arguments)
          {
          if (!is_name(arg))
            throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
          variable_names.push_back(get_name(arg));
          }
        }
      macro_entry me;
      me.name = macro_name;
      me.variables = variable_names;
      me.variable_arity = variable_arity;
      Begin b;
      b.arguments.insert(b.arguments.begin(), p.arguments.begin() + 1, p.arguments.end());
      md.m[macro_name] = me;

      Lambda lam;
      lam.variable_arity = variable_arity;
      lam.variables = variable_names;
      lam.body.push_back(b);

      PrimitiveCall prim;
      prim.primitive_name = "define";
      prim.arguments.push_back(_make_var(macro_name));
      prim.arguments.push_back(lam);

      out.expressions.push_back(prim);
      }

    for (auto& mac : mfv.lisp_defmacros)
      {
      //(defmacro id formals body ...+)
      //formals = (id ...)
      //          | id
      //          | (id ...+ . id)
      assert(std::holds_alternative<PrimitiveCall>(mac));
      auto& p = std::get<PrimitiveCall>(mac);
      if (p.arguments.size() < 3)
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_number_of_arguments);
      assert(p.arguments.size() >= 3);
      if (!std::holds_alternative<Variable>(p.arguments.front()))
        throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
      std::string macro_name = std::get<Variable>(p.arguments.front()).name;

      std::vector<std::string> variable_names;
      bool variable_arity = false;
      if (std::holds_alternative<FunCall>(p.arguments[1]))
        {
        FunCall f = std::get<FunCall>(p.arguments[1]);        
        if (!std::holds_alternative<Variable>(f.fun.front()))
          throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
        variable_names.push_back(std::get<Variable>(f.fun.front()).name);
        if (f.arguments.size() >= 2 && std::holds_alternative<Literal>(f.arguments[f.arguments.size() - 2]))
          {
          Literal& lit = std::get<Literal>(f.arguments[f.arguments.size() - 2]);
          if (std::holds_alternative<Flonum>(lit))
            {
            Flonum& fl = std::get<Flonum>(lit);
            if (fl.value != 0.0)
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
            for (size_t j = 0; j < f.arguments.size(); ++j)
              {
              if (j == (f.arguments.size() - 2))
                continue;
              if (!is_name(f.arguments[j]))
                throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
              variable_names.push_back(get_name(f.arguments[j]));
              }
            variable_arity = true;
            }
          else
            throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
          }
        else
          {
          for (const auto& arg : f.arguments)
            {
            if (!is_name(arg))
              throw_error(p.line_nr, p.column_nr, p.filename, invalid_argument);
            variable_names.push_back(get_name(arg));
            }
          }
        }
      if (std::holds_alternative<Variable>(p.arguments[1]))
        {
        variable_arity = true;
        variable_names.push_back(std::get<Variable>(p.arguments[1]).name);
        }
      macro_entry me;
      me.name = macro_name;
      me.variables = variable_names;
      me.variable_arity = variable_arity;
      Begin b;
      b.arguments.insert(b.arguments.begin(), p.arguments.begin() + 2, p.arguments.end());
      md.m[macro_name] = me;

      Lambda lam;
      lam.variable_arity = variable_arity;
      lam.variables = variable_names;
      lam.body.push_back(b);

      PrimitiveCall prim;
      prim.primitive_name = "define";
      prim.arguments.push_back(_make_var(macro_name));
      prim.arguments.push_back(lam);

      out.expressions.push_back(prim);
      }

    return out;
    }

  void compile_macros(Program& prog, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, const primitive_map& pm, const compiler_options& ops)
    {
    if (prog.expressions.empty())
      return;
    try
      {
      asmcode code;
      std::map<std::string, external_function> externals;

      compile(env, rd, md, ctxt, code, prog, pm, externals, ops); // they need to be inside the global environment, because of gc

      first_pass_data d;
      typedef uint64_t(__cdecl *_fun_ptr)(void*);

      uint64_t fie_size;
      auto fie = (_fun_ptr)assemble(fie_size, d, code);
      if (fie)
        {
        fie(&ctxt);
        md.compiled_macros.emplace_back((void*)fie, fie_size);
        }
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      throw e;
      }
    catch (std::runtime_error e)
      {
      std::cout << e.what() << "\n";
      throw e;
      }
    }

  bool expand_existing_macros(Program& prog, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, const primitive_map& pm, const compiler_options& ops)
    {
    if (!md.compiled_macros.empty())
      {
      macro_expander_visitor mev;
      mev.p_env = &env;
      mev.p_rd = &rd;
      mev.p_ctxt = &ctxt;
      mev.p_md = &md;
      mev.p_pm = &pm;
      mev.ops = ops;
      mev.macros_expanded = false;
      visitor<Program, macro_expander_visitor>::visit(prog, &mev);
      return mev.macros_expanded;
      }
    return false;
    }
  }

void expand_macros(Program& prog, environment_map& env, repl_data& rd, macro_data& md, context& ctxt, const primitive_map& pm, const compiler_options& ops)
  {
  bool macros_expanded = true;
  while (macros_expanded)
    {
    Program macro_program = get_macros(prog, md);
    compile_macros(macro_program, env, rd, md, ctxt, pm, ops);
    macros_expanded = expand_existing_macros(prog, env, rd, md, ctxt, pm, ops);
    }
  prog.macros_expanded = true;
  }

SKIWI_END
