#include "cinput_conversion.h"
#include "compile_error.h"
#include "types.h"
#include "cinput_data.h"
#include <sstream>

SKIWI_BEGIN

namespace
  {

  struct c_token
    {
    enum e_type
      {
      T_BAD,
      T_COMMA,
      T_LEFT_ROUND_BRACKET,
      T_RIGHT_ROUND_BRACKET,
      T_ID
      };

    e_type type;
    std::string value;

    c_token(e_type i_type, const std::string& v) : type(i_type), value(v) {}
    };

  void _treat_buffer(std::string& buff, std::vector<c_token>& tokens)
    {
    if (!buff.empty())
      {      
      tokens.emplace_back(c_token::T_ID, buff);        
      buff.clear();
      }
    }

  std::vector<c_token> c_tokenize(const std::string& str)
    {
    std::vector<c_token> tokens;
    std::string buff;

    const char* s = str.c_str();
    const char* s_end = str.c_str() + str.length();

    while (s < s_end)
      {
      if (*s == ' ' || *s == '\n')
        {
        _treat_buffer(buff, tokens);
        while (*s == ' ' || *s == '\n')
          {          
          ++s;
          }
        }

      const char* s_copy = s;
      switch (*s)
        {
        case '(':
        {
        _treat_buffer(buff, tokens);
        tokens.emplace_back(c_token::T_LEFT_ROUND_BRACKET, "(");
        ++s;
        break;
        }
        case ')':
        {
        _treat_buffer(buff, tokens);
        tokens.emplace_back(c_token::T_RIGHT_ROUND_BRACKET, ")");
        ++s;
        break;
        }
        case ',':
        {
        _treat_buffer(buff, tokens);
        tokens.emplace_back(c_token::T_COMMA, ",");
        ++s;
        break;
        }
        }


      if (*s_copy && (s_copy == s))
        {
        buff += *s;
        ++s;
        }
      }

    _treat_buffer(buff, tokens);
    return tokens;
    }

  bool is_c_input(const Expression& expr)
    {
    if (std::holds_alternative<FunCall>(expr))
      {
      const Expression& f = std::get<FunCall>(expr).fun.front();
      if (std::holds_alternative<Variable>(f))
        {
        if (std::get<Variable>(f).name == "c-input")
          return true;
        }
      }
    return false;
    }

  struct c_real
    {
    std::string name;
    };

  struct c_int
    {
    std::string name;
    };

  typedef std::variant<c_real, c_int> c_parameter;

  std::string current(const std::vector<c_token>& tokens)
    {
    return tokens.empty() ? std::string() : tokens.back().value;
    }

  void advance(std::vector<c_token>& tokens)
    {
    tokens.pop_back();
    }

  c_token take(std::vector<c_token>& tokens)
    {
    if (tokens.empty())
      {
      throw_error(invalid_c_input_syntax);
      }
    c_token t = tokens.back();
    tokens.pop_back();
    return t;
    }

  void require(std::vector<c_token>& tokens, std::string required)
    {
    if (tokens.empty())
      {
      throw_error(invalid_c_input_syntax);
      }
    auto t = take(tokens);
    if (t.value != required)
      {
      throw_error(invalid_c_input_syntax);
      }
    }

  std::vector<c_parameter> read_parameters(std::vector<c_token>& tokes)
    {
    std::vector<c_parameter> pars;
    if (tokes.empty())
      return pars;
    if (current(tokes) == "(")
      {
      advance(tokes);
      bool read_parameter = true;
      while (read_parameter)
        {
        auto toke = take(tokes);
        if (toke.type == c_token::T_RIGHT_ROUND_BRACKET)
          {
          return pars; // no parameters
          }
        if (toke.type != c_token::T_ID)
          throw_error(invalid_c_input_syntax);
        c_parameter par;
        switch (toke.value.front())
          {
          case 'd':
            if (toke.value == "double")
              {
              c_real fp;             
              toke = take(tokes);
              if (toke.type != c_token::T_ID)
                throw_error(invalid_c_input_syntax);
              fp.name = toke.value;
              par = fp;
              break;
              }
          case 'i':
            if (toke.value == "int")
              {
              c_int ip;             
              toke = take(tokes);
              if (toke.type != c_token::T_ID)
                throw_error(invalid_c_input_syntax);
              ip.name = toke.value;
              par = ip;
              break;
              }
          default: throw_error(invalid_c_input_syntax);
          }
        pars.push_back(par);
        if (current(tokes) == ")")
          read_parameter = false;
        else
          require(tokes, ",");
        }
      require(tokes, ")");
      }
    return pars;
    }

  std::string make_name(const std::string& original, uint64_t i)
    {
    std::stringstream str;
    str << original << "_" << i;
    return str.str();
    }

  void read_c_input(cinput_data& cinput, const Expression& expr, environment_map& env, repl_data& rd)
    {
    const Expression& pars = std::get<FunCall>(expr).arguments.front();
    const Literal& lit = std::get<Literal>(pars);
    const String& s = std::get<String>(lit);
    auto tokens = c_tokenize(s.value);
    std::reverse(tokens.begin(), tokens.end());
    auto parsed = read_parameters(tokens);
    
    if (parsed.empty())
      return;
    
    std::shared_ptr < environment<alpha_conversion_data>> alpha_env = std::make_shared<environment<alpha_conversion_data>>(rd.alpha_conversion_env);

    for (const auto p : parsed)
      {
      if (std::holds_alternative<c_int>(p))
        {
        std::string name = std::get<c_int>(p).name;
        std::string newname = make_name(name, rd.alpha_conversion_index++);
        alpha_env->push(name, newname);
        environment_entry ne;
        ne.st = environment_entry::st_global;
        ne.pos = rd.global_index * 8;
        ++(rd.global_index);
        env->push(newname, ne);
        cinput.parameters.emplace_back(newname, cinput_data::cin_int);
        }
      else if (std::holds_alternative<c_real>(p))
        {
        std::string name = std::get<c_real>(p).name;
        std::string newname = make_name(name, rd.alpha_conversion_index++);
        alpha_env->push(name, newname);
        environment_entry ne;
        ne.st = environment_entry::st_global;
        ne.pos = rd.global_index * 8;
        ++(rd.global_index);
        env->push(newname, ne);
        cinput.parameters.emplace_back(newname, cinput_data::cin_double);
        }
      }
    rd.alpha_conversion_env = alpha_env;
    }
  }

void cinput_conversion(cinput_data& cinput, Program& prog, environment_map& env, repl_data& rd)
  {
  cinput.parameters.clear();
  if (!prog.expressions.empty())
    {
    if (is_c_input(prog.expressions.front()))
      {
      read_c_input(cinput, prog.expressions.front(), env, rd);
      prog.expressions.erase(prog.expressions.begin());
      }
    }
  }

SKIWI_END