#include "parse.h"
#include "tokenize.h"
#include "compile_error.h"
#include <sstream>
#include <inttypes.h>
#include <stdio.h>
#include <map>

SKIWI_BEGIN



std::map<std::string, expression_type> generate_expression_map()
  {
  std::map<std::string, expression_type> m;
  m.insert(std::pair<std::string, expression_type>("##remainder", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##quotient", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##arithmetic-shift", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##%quiet-undefined", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##%undefined", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##vector-length", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fixnum->flonum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flonum->fixnum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fixnum->char", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char->fixnum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char<?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char>?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char<=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char>=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##bitwise-and", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##bitwise-not", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##bitwise-or", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##bitwise-xor", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-sign", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-mantissa", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-exponent", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-pi", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxsin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxcos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxtan", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxasin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxacos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxatan1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxlog", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxround", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxtruncate", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fxsqrt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flsin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flcos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fltan", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flasin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flacos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flatan1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fllog", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flround", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-fltruncate", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##ieee754-flsqrt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##memq", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##assq", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##not", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##cons", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##car", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##cdr", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##set-car!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##set-cdr!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##eq?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fxadd1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fxsub1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fxmax", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fxmin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fxzero?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx+", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx-", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx*", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx/", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx<?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx>?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx<=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx>=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fx!=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fladd1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flsub1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flmax", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flmin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flzero?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl+", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl-", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl*", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl/", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl<?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl>?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl<=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl>=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fl!=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##fixnum?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##flonum?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##pair?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##vector?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##string?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##symbol?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##closure?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##procedure?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##boolean?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##null?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##eof-object?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##char?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##promise?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##port?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##input-port?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("##output-port?", et_primitive_call));

  m.insert(std::pair<std::string, expression_type>("+", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("-", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("*", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("/", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("=", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("!=", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("<", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("<=", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>(">", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>(">=", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%allocate-symbol", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%slot-ref", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%slot-set!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%quiet-undefined", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%undefined", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("add1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("and", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%apply", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("arithmetic-shift", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("assoc", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("assv", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("assq", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("bitwise-and", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("bitwise-not", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("bitwise-or", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("bitwise-xor", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("boolean?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("car", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("cdr", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char->fixnum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char>?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char<?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char>=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("char<=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("close-file", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("closure", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("closure?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("closure-ref", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("cons", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("compare-strings", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("compare-strings-ci", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("define", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("defmacro", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("define-macro", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("delay", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("eq?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("eqv?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%eqv?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("eof-object?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("equal?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%error", et_primitive_call));  
  m.insert(std::pair<std::string, expression_type>("%eval", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("file-exists?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fixnum->char", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fixnum->flonum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fixnum?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fixnum-expt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("flonum?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("flonum->fixnum", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("flonum-expt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%flush-output-port", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx>?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx<?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx>=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx<=?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx+", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx-", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx*", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fx/", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fxadd1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fxsub1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("fxzero?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("getenv", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("halt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-sign", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-exponent", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-mantissa", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-sin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-cos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-tan", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-asin", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-acos", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-atan1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-log", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-round", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-sqrt", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-truncate", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("ieee754-pi", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("include", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("input-port?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("load", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("length", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("list", et_primitive_call));  
  m.insert(std::pair<std::string, expression_type>("make-port", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%make-promise", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("make-string", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("make-vector", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("max", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("member", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("memv", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("memq", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("min", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("not", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("null?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("num2str", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("or", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("open-file", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("output-port?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("pair?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%peek-char", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("port?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("procedure?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("promise?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("putenv", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("quotient", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%read-char", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("reclaim", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("reclaim-garbage", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("remainder", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("set-car!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("set-cdr!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("str2num", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-append1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-copy", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-fill!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-hash", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-length", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-ref", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("string-set!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("substring", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("symbol?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("symbol->string", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("sub1", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("unless", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector?", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector-fill!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector-length", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector-ref", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("vector-set!", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("when", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%write-char", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("%write-string", et_primitive_call));
  m.insert(std::pair<std::string, expression_type>("zero?", et_primitive_call));

  m.insert(std::pair<std::string, expression_type>("begin", et_begin));
  m.insert(std::pair<std::string, expression_type>("case", et_case));
  m.insert(std::pair<std::string, expression_type>("cond", et_cond));
  m.insert(std::pair<std::string, expression_type>("do", et_do));
  m.insert(std::pair<std::string, expression_type>("foreign-call", et_foreign_call));
  m.insert(std::pair<std::string, expression_type>("if", et_if));
  m.insert(std::pair<std::string, expression_type>("lambda", et_lambda));
  m.insert(std::pair<std::string, expression_type>("let", et_let));
  m.insert(std::pair<std::string, expression_type>("letrec", et_letrec));
  m.insert(std::pair<std::string, expression_type>("let*", et_let_star));
  m.insert(std::pair<std::string, expression_type>("quote", et_quote));
  m.insert(std::pair<std::string, expression_type>("quasiquote", et_quasiquote));
  m.insert(std::pair<std::string, expression_type>("set!", et_set));
  return m;
  }

namespace
  {


  uint64_t hex_to_uint64_t(const std::string& h)
    {
    uint64_t out = 0;
    if (h.empty())
      return out;
    int i = (int)h.size() - 1;
    int j = 0;
    for (; i >= 0; --i)
      {
      int v = h[i] >= 'A' ? (h[i] >= 'a' ? ((int)h[i] - 87) : ((int)h[i] - 55)) : ((int)h[i] - 48);
      if (v > 15 || v < 0)
        throw_error(unsupported_number_syntax);
      out |= (((uint64_t)v) << (4 * j));
      ++j;
      }

    return out;
    }

  uint64_t binary_to_uint64_t(const std::string& h)
    {
    uint64_t out = 0;
    if (h.empty())
      return out;
    int i = (int)h.size() - 1;
    int j = 0;
    for (; i >= 0; --i)
      {
      int v = (int)h[i] - 48;
      if (v != 0 && v != 1)
        throw_error(unsupported_number_syntax);
      out |= (((uint64_t)v) << j);
      ++j;
      }

    return out;
    }

  uint64_t octal_to_uint64_t(const std::string& h)
    {
    uint64_t out = 0;
    if (h.empty())
      return out;
    int i = (int)h.size() - 1;
    int j = 0;
    for (; i >= 0; --i)
      {
      int v = (int)h[i] - 48;
      if (v < 0 || v > 7)
        throw_error(unsupported_number_syntax);
      out |= (((uint64_t)v) << (3*j));
      ++j;
      }

    return out;
    }

  bool find_expression_type(expression_type& t, const std::string& token, const std::map<std::string, expression_type>& m)
    {
    auto it = m.find(token);
    if (it != m.end())
      {
      t = it->second;
      return true;
      }
    return false;
    }

  void throw_parse_error(int line_nr, int column_nr, error_type t, std::string extra = std::string(""))
    {
    throw_error(line_nr, column_nr, "", t, extra);
    }

  void throw_parse_error(error_type t)
    {
    throw_parse_error(-1, -1, t);
    }

  token popped_token = token(token::T_BAD, "", -1, -1);

  void invalidate_popped()
    {
    popped_token = token(token::T_BAD, "", -1, -1);
    }

  std::string current(const std::vector<token>& tokens)
    {
    return tokens.empty() ? std::string() : tokens.back().value;
    }

  token::e_type current_type(const std::vector<token>& tokens)
    {
    return tokens.empty() ? token::T_BAD : tokens.back().type;
    }

  int current_line(const std::vector<token>& tokens)
    {
    return tokens.empty() ? -1 : tokens.back().line_nr;
    }

  token take(std::vector<token>& tokens)
    {
    if (tokens.empty())
      {
      throw_parse_error(no_tokens);
      }
    token t = tokens.back();
    popped_token = t;
    tokens.pop_back();
    return t;
    }

  void advance(std::vector<token>& tokens)
    {
    popped_token = tokens.back();
    tokens.pop_back();
    }

  bool is_next(const std::vector<token>& tokens, std::string expected)
    {
    return tokens.size() < 2 ? false : tokens[tokens.size() - 2].value == expected;
    }

  void require(std::vector<token>& tokens, std::string required)
    {
    if (tokens.empty())
      {
      throw_parse_error(-1, -1, expected_keyword, required);
      }
    auto t = take(tokens);
    if (t.value != required)
      {
      throw_parse_error(t.line_nr, t.column_nr, expected_keyword, required);
      }
    }

  bool closing_brackets(const std::vector<token>& tokens)
    {
    return (current_type(tokens) == token::T_RIGHT_ROUND_BRACKET) || (current_type(tokens) == token::T_RIGHT_SQUARE_BRACKET);
    }

  bool require_left_square_bracket(std::vector<token>& tokens)
    {
    if (tokens.empty())
      {
      throw_parse_error(-1, -1, expected_keyword, "[");
      }
    auto t = take(tokens);
    bool res = (t.type == token::T_LEFT_SQUARE_BRACKET);
    if (!(res || (t.type == token::T_LEFT_ROUND_BRACKET)))
      {
      throw_parse_error(t.line_nr, t.column_nr, expected_keyword, "[");
      }
    invalidate_popped();
    return res;
    }

  void require_right_square_bracket(bool result_of_require_left_square_bracket, std::vector<token>& tokens)
    {
    if (tokens.empty())
      {
      throw_parse_error(-1, -1, expected_keyword, "]");
      }
    auto t = take(tokens);
    bool success = (result_of_require_left_square_bracket && (t.type == token::T_RIGHT_SQUARE_BRACKET)) || (!result_of_require_left_square_bracket && (t.type == token::T_RIGHT_ROUND_BRACKET));
    if (!success)
      {
      throw_parse_error(t.line_nr, t.column_nr, expected_keyword, result_of_require_left_square_bracket ? "]" : ")");
      }
    }

  int64_t s64(const char *s)
    {
    uint64_t i;
    char c;
    sscanf(s, "%" SCNu64 "%c", &i, &c);
    return (int64_t)i;
    }
  } // anonymous namespace

PrimitiveCall make_primitive_call(std::vector<token>& tokens)
  {
  PrimitiveCall p;
  if (popped_token.type != token::T_LEFT_ROUND_BRACKET)
    {
    if (tokens.empty())
      throw_parse_error(no_tokens);
    if (current_type(tokens) != token::T_ID)
      throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
    p.line_nr = tokens.back().line_nr;
    p.column_nr = tokens.back().column_nr;
    p.primitive_name = current(tokens);
    advance(tokens);
    p.as_object = true;
    return p;
    }
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  p.line_nr = tokens.back().line_nr;
  p.column_nr = tokens.back().column_nr;
  p.primitive_name = current(tokens);
  if (popped_token.type != token::T_LEFT_ROUND_BRACKET)
    {
    advance(tokens);
    return p;
    }
  advance(tokens);
  while (current(tokens) != ")")
    {
    p.arguments.push_back(make_expression(tokens));
    if (tokens.empty())
      throw_parse_error(p.line_nr, p.column_nr, expected_keyword, ")");
    }
  return p;
  }

ForeignCall make_foreign_call(std::vector<token>& tokens)
  {
  ForeignCall p;
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  std::string name = current(tokens);
  if (name != "foreign-call")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "foreign-call");
  p.line_nr = tokens.back().line_nr;
  p.column_nr = tokens.back().column_nr;
  advance(tokens);
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  p.foreign_name = current(tokens);
  advance(tokens);
  while (current(tokens) != ")")
    {
    p.arguments.push_back(make_expression(tokens));
    if (tokens.empty())
      throw_parse_error(p.line_nr, p.column_nr, expected_keyword, ")");
    }
  return p;
  }

Do make_do(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Do d;
  d.line_nr = tokens.back().line_nr;
  d.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "do")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "do");
  advance(tokens);
  require(tokens, "(");
  while (current(tokens) != ")")
    {
    //require(tokens, "[");
    bool rlsqb = require_left_square_bracket(tokens);
    Variable var = make_variable(tokens);
    Expression init = make_expression(tokens);
    std::vector<Expression> exprs;
    if (closing_brackets(tokens))
      {
      exprs.push_back(var);
      exprs.push_back(init);
      exprs.push_back(var);
      }
    else
      {
      Expression step = make_expression(tokens);
      exprs.push_back(var);
      exprs.push_back(init);
      exprs.push_back(step);
      }
    d.bindings.push_back(exprs);
    //require(tokens, "]");
    require_right_square_bracket(rlsqb, tokens);
    }
  advance(tokens);
  require(tokens, "(");
  invalidate_popped();
  while (current(tokens) != ")")
    {
    d.test.push_back(make_expression(tokens));
    }
  if (d.test.size() == 1) // only test, no expressions
    d.test.push_back(Nop());
  advance(tokens);
  while (current(tokens) != ")")
    {
    d.commands.push_back(make_expression(tokens));
    }
  return d;
  }

Case make_case(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Case c;
  c.line_nr = tokens.back().line_nr;
  c.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "case")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "case");
  advance(tokens);
  c.val_expr.push_back(make_expression(tokens));
  if (current(tokens) == ")")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "[");
  bool else_statement = false;
  while (current(tokens) != ")")
    {
    if (else_statement)
      throw_parse_error(c.line_nr, c.column_nr, expected_keyword, ")");
    //require(tokens, "[");
    bool rlsb = require_left_square_bracket(tokens);
    while (!closing_brackets(tokens))
      {
      if (current(tokens) == "else")
        {
        advance(tokens);
        else_statement = true;
        while (!closing_brackets(tokens))
          {
          c.else_body.push_back(make_expression(tokens));
          }
        }
      else
        {
        c.datum_args.emplace_back();
        c.then_bodies.emplace_back();
        c.datum_args.back() = read_from(tokens);
        if (c.datum_args.back().type != ct_pair)
          throw_parse_error(c.line_nr, c.column_nr, bad_syntax);
        while (!closing_brackets(tokens))
          {
          c.then_bodies.back().push_back(make_expression(tokens));
          }
        }
      }
    //advance(tokens);
    require_right_square_bracket(rlsb, tokens);
    }
  if (!else_statement)
    c.else_body.push_back(Nop());
  return c;
  }

Cond make_cond(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Cond c;
  c.line_nr = tokens.back().line_nr;
  c.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "cond")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "cond");
  advance(tokens);
  bool else_statement = false;
  while (current(tokens) != ")")
    {
    c.is_proc.push_back(false);
    if (else_statement)
      throw_parse_error(c.line_nr, c.column_nr, expected_keyword, ")");
    std::vector<Expression> expr;
    //require(tokens, "[");
    bool rlsb = require_left_square_bracket(tokens);
    while (!closing_brackets(tokens))
      {
      if (current(tokens) == "else")
        {
        advance(tokens);
        else_statement = true;
        expr.push_back(Expression(Literal(True())));
        }
      else
        {
        if (current(tokens) == "=>")
          {
          c.is_proc.back() = true;
          advance(tokens);
          if (tokens.empty())
            throw_parse_error(c.line_nr, c.column_nr, no_tokens);
          expr.push_back(make_expression(tokens));
          if (!closing_brackets(tokens))
            throw_parse_error(c.line_nr, c.column_nr, expected_keyword, "]");
          }
        else
          {
          expr.push_back(make_expression(tokens));
          if (tokens.empty())
            throw_parse_error(c.line_nr, c.column_nr, expected_keyword, "]");
          }
        }
      }
    //advance(tokens);
    require_right_square_bracket(rlsb, tokens);
    c.arguments.push_back(expr);
    }
  return c;
  }

If make_if(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  If i;
  i.line_nr = tokens.back().line_nr;
  i.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "if")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "if");
  advance(tokens);
  while (current(tokens) != ")")
    {
    i.arguments.push_back(make_expression(tokens));
    if (tokens.empty())
      throw_parse_error(i.line_nr, i.column_nr, expected_keyword, ")");
    }
  if (i.arguments.size() < 2 || i.arguments.size() > 3)
    throw_parse_error(i.line_nr, i.column_nr, invalid_number_of_arguments);
  if (i.arguments.size() == 2)
    i.arguments.push_back(Nop());
  return i;
  }

Literal make_literal(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  switch (current_type(tokens))
    {
    case token::T_LEFT_ROUND_BRACKET:
    {
    Nil n;
    n.line_nr = tokens.back().line_nr;
    n.column_nr = tokens.back().column_nr;
    advance(tokens);
    require(tokens, ")");
    return n;
    }
    case token::T_FIXNUM:
    {
    Fixnum f;
    f.line_nr = tokens.back().line_nr;
    f.column_nr = tokens.back().column_nr;
    f.value = s64(take(tokens).value.c_str());
    return f;
    }
    case token::T_FLONUM:
    {
    Flonum f;
    f.line_nr = tokens.back().line_nr;
    f.column_nr = tokens.back().column_nr;
    f.value = atof(take(tokens).value.c_str());
    return f;
    }
    case token::T_STRING:
    {
    String s;
    s.line_nr = tokens.back().line_nr;
    s.column_nr = tokens.back().column_nr;
    auto str = take(tokens).value;
    s.value = str.substr(1, str.size() - 2);
    return s;
    }
    case token::T_SYMBOL:
    {
    int ln = tokens.back().line_nr;
    int cn = tokens.back().column_nr;
    auto c = current(tokens);
    if (c == "#t")
      {
      advance(tokens);
      True t;
      t.line_nr = ln;
      t.column_nr = cn;
      return t;
      }
    else if (c == "#f")
      {
      advance(tokens);
      False f;
      f.line_nr = ln;
      f.column_nr = cn;
      return f;
      }
    else if (c.size() >= 2 && c[0] == '#' && c[1] == '\\')
      {
      std::string s = take(tokens).value.substr(2);
      Character ch;
      ch.line_nr = ln;
      ch.column_nr = cn;
      if (s.size() == 1)
        {
        ch.value = s[0];
        return ch;
        }
      else if (!s.empty())
        {
        switch (s[0])
          {
          case 'b': if (s == "backspace") { ch.value = 8; return ch; } break;
          case 't': if (s == "tab") { ch.value = 9; return ch; } break;
          case 'n': if (s == "newline") { ch.value = 10; return ch; } break;
          case 'l': if (s == "linefeed") { ch.value = 10; return ch; } break;
          case 'v': if (s == "vtab") { ch.value = 11; return ch; } break;
          case 'p': if (s == "page") { ch.value = 12; return ch; } break;
          case 'r': if (s == "return") { ch.value = 13; return ch; } if (s == "rubout") { ch.value = 127; return ch; } break;
          case 's': if (s == "space") { ch.value = 32; return ch; } break;
          }
        }
      else if (s.empty())
        {
        ch.value = 32;
        return ch;
        }
      ch.value = char(atoi(s.c_str()));
      return ch;
      }
    else if (c.size() >= 2 && c[0] == '#' && c[1] == 'x')
      {
      std::string hex = take(tokens).value.substr(2);
      uint64_t fixnum = hex_to_uint64_t(hex);
      Fixnum f;
      f.line_nr = ln;
      f.column_nr = cn;
      f.value = (int64_t)fixnum;
      return f;
      }
    else if (c.size() >= 2 && c[0] == '#' && c[1] == 'b')
      {
      std::string hex = take(tokens).value.substr(2);
      uint64_t fixnum = binary_to_uint64_t(hex);
      Fixnum f;
      f.line_nr = ln;
      f.column_nr = cn;
      f.value = (int64_t)fixnum;
      return f;
      }
    else if (c.size() >= 2 && c[0] == '#' && c[1] == 'o')
      {
      std::string hex = take(tokens).value.substr(2);
      uint64_t fixnum = octal_to_uint64_t(hex);
      Fixnum f;
      f.line_nr = ln;
      f.column_nr = cn;
      f.value = (int64_t)fixnum;
      return f;
      }
    }
    }
  throw_parse_error(not_implemented);
  return Nil();
  }

Lambda make_lambda(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Lambda l;
  l.line_nr = tokens.back().line_nr;
  l.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  l.variable_arity = false;
  if (name != "lambda")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "lambda");
  advance(tokens);
  if (current(tokens) != "(") // rest argument
    {
    l.variable_arity = true;
    auto tok = take(tokens);
    l.variables.emplace_back(tok.value);
    }
  else
    {
    require(tokens, "(");
    int count_after_var = 0;
    while (current(tokens) != ")")
      {
      if (l.variable_arity)
        ++count_after_var;
      auto tok = take(tokens);
      if (tok.type != token::T_ID && tok.value != ".")
        throw_parse_error(tok.line_nr, tok.column_nr, invalid_argument);
      if (tok.value == ".")
        {
        if (l.variable_arity)
          throw_parse_error(tok.line_nr, tok.column_nr, invalid_argument);
        l.variable_arity = true;
        }
      else
        l.variables.emplace_back(tok.value);
      }
    if (l.variable_arity && (count_after_var != 1))
      throw_parse_error(l.line_nr, l.column_nr, invalid_argument);
    advance(tokens);
    }
  Begin b;
  b = make_begin(tokens, true);
  while (b.arguments.size() == 1 && std::holds_alternative<Begin>(b.arguments.front()))
    {
    Begin beg = std::get<Begin>(b.arguments.front());
    b = beg;
    }
  l.body.push_back(b);
  return l;
  }

Let make_let(std::vector<token>& tokens, binding_type t)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Let l;
  l.line_nr = tokens.back().line_nr;
  l.column_nr = tokens.back().column_nr;
  l.bt = t;
  std::string name = current(tokens);
  std::string letname = "let";
  switch (t)
    {
    case bt_let:
      letname = "let";
      break;
    case bt_letrec:
      letname = "letrec";
      break;
    case bt_let_star:
      letname = "let*";
      break;
    default:
      throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, not_implemented);
    }
  if (name != letname)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, letname);
  advance(tokens);

  if (current_type(tokens) == token::T_ID) // named let?
    {
    if (l.bt != bt_let)
      throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "(");
    l.named_let = true;
    l.let_name = current(tokens);
    advance(tokens);
    }

  require(tokens, "(");
  while (current(tokens) != ")")
    {
    //require(tokens, "[");
    bool rlsb = require_left_square_bracket(tokens);
    auto tok = take(tokens);
    if (tok.type != token::T_ID)
      throw_parse_error(tok.line_nr, tok.column_nr, invalid_argument);
    Expression expr = make_expression(tokens);
    l.bindings.emplace_back(tok.value, expr);
    //require(tokens, "]");
    require_right_square_bracket(rlsb, tokens);
    }
  advance(tokens);
  Begin b;
  b = make_begin(tokens, true);
  while (b.arguments.size() == 1 && std::holds_alternative<Begin>(b.arguments.front()))
    {
    Begin beg = std::get<Begin>(b.arguments.front());
    b = beg;
    }
  l.body.push_back(b);
  return l;
  }

Variable make_variable(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  Variable v;
  v.line_nr = tokens.back().line_nr;
  v.column_nr = tokens.back().column_nr;
  v.name = current(tokens);
  advance(tokens);
  return v;
  }

Begin make_begin(std::vector<token>& tokens, bool implicit)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  Begin b;
  b.line_nr = tokens.back().line_nr;
  b.column_nr = tokens.back().column_nr;
  if (!implicit)
    {
    if (current_type(tokens) != token::T_ID)
      throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
    std::string name = current(tokens);
    if (name != "begin")
      throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "begin");
    advance(tokens);
    }
  invalidate_popped();
  while (current(tokens) != ")")
    {
    b.arguments.push_back(make_expression(tokens));
    if (tokens.empty())
      throw_parse_error(b.line_nr, b.column_nr, expected_keyword, ")");
    }
  return b;
  }

Set make_set(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
  std::string name = current(tokens);
  Set s;
  s.line_nr = tokens.back().line_nr;
  s.column_nr = tokens.back().column_nr;
  if (name != "set!")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "set!");
  advance(tokens);
  auto tok = take(tokens);
  if (tok.type != token::T_ID)
    throw_parse_error(tok.line_nr, tok.column_nr, invalid_variable_name);
  s.name = tok.value;
  s.value.push_back(make_expression(tokens));
  return s;
  }

Quote make_unquote(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID && current_type(tokens) != token::T_UNQUOTE)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax, "`");
  Quote q;
  q.type = Quote::qt_unquote;
  q.line_nr = tokens.back().line_nr;
  q.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "unquote" && name != ",")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "unquote");
  advance(tokens);
  q.arg = read_from(tokens, true);
  return q;
  }

Quote make_unquote_splicing(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID && current_type(tokens) != token::T_UNQUOTE_SPLICING)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax, "`");
  Quote q;
  q.type = Quote::qt_unquote_splicing;
  q.line_nr = tokens.back().line_nr;
  q.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "unquote-splicing" && name != ",@")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "unquote-splicing");
  advance(tokens);
  q.arg = read_from(tokens, true);
  return q;
  }

Quote make_quasiquote(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID && current_type(tokens) != token::T_BACKQUOTE)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax, "`");
  Quote q;
  q.type = Quote::qt_backquote;
  q.line_nr = tokens.back().line_nr;
  q.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "quasiquote" && name != "`")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "quasiquote");
  advance(tokens);
  q.arg = read_from(tokens, true);
  return q;
  }

Quote make_quote(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  if (current_type(tokens) != token::T_ID && current_type(tokens) != token::T_QUOTE)
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax, "'");
  Quote q;
  q.line_nr = tokens.back().line_nr;
  q.column_nr = tokens.back().column_nr;
  std::string name = current(tokens);
  if (name != "quote" && name != "'")
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, expected_keyword, "quote");
  advance(tokens);
  q.arg = read_from(tokens);
  return q;
  }

FunCall make_fun(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);
  FunCall f;
  f.line_nr = tokens.back().line_nr;
  f.column_nr = tokens.back().column_nr;
  f.fun.push_back(make_expression(tokens));
  while (current(tokens) != ")")
    {
    f.arguments.push_back(make_expression(tokens));
    if (tokens.empty())
      throw_parse_error(f.line_nr, f.column_nr, expected_keyword, ")");
    }
  return f;
  }

Expression make_expression(std::vector<token>& tokens)
  {
  if (tokens.empty())
    throw_parse_error(no_tokens);

  static std::map<std::string, expression_type> expression_map = generate_expression_map();

  switch (current_type(tokens))
    {
    case token::T_LEFT_ROUND_BRACKET:
    {
    if (is_next(tokens, ")")) // nil
      return make_literal(tokens);
    bool function_call = (popped_token.type == token::T_LEFT_ROUND_BRACKET);
    int ln = tokens.back().line_nr;
    int cn = tokens.back().column_nr;
    advance(tokens);
    if (function_call)
      {
      FunCall f;
      f.line_nr = ln;
      f.column_nr = cn;
      f.fun.push_back(make_expression(tokens));
      require(tokens, ")");
      if (current(tokens) != ")")
        {
        while (current(tokens) != ")")
          {
          f.arguments.push_back(make_expression(tokens));
          if (tokens.empty())
            throw_parse_error(ln, cn, expected_keyword, ")");
          }
        }
      if (f.arguments.empty() && std::holds_alternative<Literal>(f.fun.front()))
        {
        return f.fun.front();
        }
      return f;
      }
    else
      {
      auto expr = make_expression(tokens);
      if (current(tokens) != ")")
        {
        FunCall f;
        f.line_nr = ln;
        f.column_nr = cn;
        f.fun.push_back(expr);
        while (current(tokens) != ")")
          {
          f.arguments.push_back(make_expression(tokens));
          if (tokens.empty())
            throw_parse_error(ln, cn, expected_keyword, ")");
          }
        require(tokens, ")");
        return f;
        }
      else
        require(tokens, ")");
      return expr;
      }
    }
    case token::T_RIGHT_ROUND_BRACKET:
    {
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
    }
    case token::T_BAD:
    {
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, bad_syntax);
    }
    case token::T_FIXNUM:
    {
    return make_literal(tokens);
    }
    case token::T_FLONUM:
    {
    return make_literal(tokens);
    }
    case token::T_STRING:
    {
    return make_literal(tokens);
    }
    case token::T_SYMBOL:
    {
    if (current(tokens) == "#undefined")
      return Nop();
    return make_literal(tokens);
    }
    case token::T_QUOTE:
    {
    return make_quote(tokens);
    }
    case token::T_BACKQUOTE:
    {
    return make_quasiquote(tokens);
    }
    case token::T_UNQUOTE:
    {
    return make_unquote(tokens);
    }
    case token::T_UNQUOTE_SPLICING:
    {
    return make_unquote_splicing(tokens);
    }
    case token::T_ID:
    {
    std::string curr = current(tokens);
    expression_type t;
    if (find_expression_type(t, curr, expression_map))
      {
      switch (t)
        {
        case et_begin: return make_begin(tokens, false);
        case et_case: return make_case(tokens);
        case et_cond: return make_cond(tokens);
        case et_do: return make_do(tokens);
        case et_foreign_call: return make_foreign_call(tokens);
        case et_if: return make_if(tokens);
        case et_lambda: return make_lambda(tokens);
        case et_let: return make_let(tokens, bt_let);
        case et_letrec: return make_let(tokens, bt_letrec);
        case et_let_star: return make_let(tokens, bt_let_star);
        case et_quote: return make_quote(tokens);
        case et_quasiquote: return make_quasiquote(tokens);
        case et_unquote: return make_unquote(tokens);
        case et_unquote_splicing: return make_unquote_splicing(tokens);
        case et_primitive_call: return make_primitive_call(tokens);
        case et_set: return make_set(tokens);
        default: throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, not_implemented);
        }
      }
    if (popped_token.type == token::T_LEFT_ROUND_BRACKET)
      {
      invalidate_popped();
      return make_fun(tokens);
      }
    return make_variable(tokens);
    }
    default:
    {
    throw_parse_error(tokens.back().line_nr, tokens.back().column_nr, not_implemented);
    }
    }
  throw_parse_error(not_implemented);
  return Nop();
  }

Program make_program(std::vector<token>& tokens)
  {
  Program prog;
  while (!tokens.empty())
    prog.expressions.push_back(make_expression(tokens));
  return prog;
  }

SKIWI_END
