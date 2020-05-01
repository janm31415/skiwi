#pragma once

#include "namespace.h"

#include "reader.h"
#include "libskiwi_api.h"
#include "tokenize.h"
#include "liveness_range.h"

#include <stdint.h>
#include <variant>
#include <vector>
#include <set>
#include <map>
#include <string>

SKIWI_BEGIN

enum expression_type
  {
  et_primitive_call,
  et_begin,
  et_case,
  et_cond,
  et_do,
  et_foreign_call,
  et_if,
  et_lambda,
  et_let,
  et_letrec,
  et_let_star,
  et_quote,
  et_quasiquote,
  et_unquote,
  et_unquote_splicing,
  et_set
  };

std::map<std::string, expression_type> generate_expression_map();

struct Fixnum
  {
  Fixnum() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  int64_t value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Flonum
  {
  Flonum() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  double value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Nil
  {
  Nil() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct String
  {
  String() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::string value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct True
  {
  True() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct False
  {
  False() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Character
  {
  Character() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  char value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Symbol
  {
  Symbol() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::string value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

typedef std::variant<Character, False, Fixnum, Flonum, Nil, String, Symbol, True> Literal;

struct Variable
  {
  Variable() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::string name;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Nop
  {
  Nop() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct ForeignCall;
struct PrimitiveCall;
struct If;
struct Let;
struct Lambda;
struct FunCall;
struct Begin;
struct Set;
struct Quote;
struct Case;
struct Cond;
struct Do;

typedef std::variant<Begin, Case, Cond, Do, ForeignCall, FunCall, If, Lambda, Let, Literal, Nop, PrimitiveCall, Quote, Set, Variable> Expression;

struct PrimitiveCall
  {
  PrimitiveCall() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0), as_object(false) {}
  int line_nr, column_nr;
  std::string primitive_name;
  std::vector<Expression> arguments;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  bool as_object; // primitives can be passed as object
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct ForeignCall
  {
  ForeignCall() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::string foreign_name;
  std::vector<Expression> arguments;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct If
  {
  If() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<Expression> arguments;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Cond
  {
  Cond() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<std::vector<Expression>> arguments;
  std::vector<bool> is_proc;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Do
  {
  Do() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<std::vector<Expression>> bindings; // [<variable1> <init1> <step1>] ...
  std::vector<Expression> test; // <test> <expr> ...
  std::vector<Expression> commands; // <command> ...  
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Case
  {
  Case() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<Expression> val_expr;
  std::vector<cell> datum_args;
  std::vector<std::vector<Expression>> then_bodies;
  std::vector<Expression> else_body;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Quote
  {
  enum quote_type
    {
    qt_quote,
    qt_backquote,
    qt_unquote,
    qt_unquote_splicing
    };
  Quote() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0), type(qt_quote) {}
  int line_nr, column_nr;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  cell arg;
  quote_type type;
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

enum binding_type
  {
  bt_let,
  bt_letrec,
  bt_let_star
  };

struct Let
  {
  Let() : bt(bt_let), line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0), named_let(false) {}
  int line_nr, column_nr;
  std::vector<Expression> body;
  std::vector<std::pair<std::string, Expression>> bindings;
  binding_type bt;
  std::set<std::string> assignable_variables;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::vector<liveness_range> live_ranges; // for each variable contains the intervals of time points where the variable is live
  bool named_let;
  std::string let_name;
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Lambda
  {
  Lambda() : line_nr(-1), column_nr(-1), variable_arity(false), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  bool variable_arity;
  std::vector<std::string> variables;
  std::vector<Expression> body;
  std::vector<std::string> free_variables;
  std::set<std::string> assignable_variables;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::vector<liveness_range> live_ranges; // for each variable contains the intervals of time points where the variable is live
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct FunCall
  {
  FunCall() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<Expression> arguments;
  std::vector<Expression> fun;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Begin
  {
  Begin() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0) {}
  int line_nr, column_nr;
  std::vector<Expression> arguments;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

struct Set
  {
  Set() : line_nr(-1), column_nr(-1), tail_position(false), scan_index(0), pre_scan_index(0), originates_from_define(false), originates_from_quote(false) {}
  int line_nr, column_nr;
  std::string name;
  std::vector<Expression> value;
  bool tail_position; // true if expr is in tail position. boolean is set by tail_call_analysis.
  bool originates_from_define; // external defines are rewritten as set! by define_conversion. if so, originates_from_define will be true.
  bool originates_from_quote; // new quotes are rewritten as set! by quote_conversion. if so, originates_from_quote will be true.
  uint64_t pre_scan_index, scan_index; // indicates program time point for linear scanning algorithm to compute liveness of variables
  std::string filename; // the name of the file where this expression is read (if load is used, empty otherwise)
  };

typedef std::vector<Expression> Expressions;

class Program
  {
  public:
    Program()
      {
      alpha_converted = false;
      assignable_variables_converted = false;
      define_converted = false;
      closure_converted = false;
      cps_converted = false;
      free_variables_analysed = false;
      linear_scan_indices_computed = false;
      simplified_to_core_forms = false;
      single_begin_conversion = false;
      tail_call_analysis = false;
      global_define_env_allocated = false;
      quotes_collected = false;
      quotes_converted = false;      
      quasiquotes_converted = false;
      include_handled = false;
      lambda_to_let_converted = false;
      inline_primitives_converted = false;
      constant_folded = false;
      constant_propagated = false;
      macros_expanded = false;
      }
    Expressions expressions;
    std::map<std::string, cell> quotes;

    bool alpha_converted;
    bool assignable_variables_converted;
    bool define_converted;
    bool closure_converted;
    bool cps_converted;    
    bool free_variables_analysed;
    bool linear_scan;
    bool linear_scan_indices_computed;
    bool lambda_to_let_converted;
    bool include_handled;
    bool simplified_to_core_forms;     
    bool single_begin_conversion;
    bool tail_call_analysis;
    bool global_define_env_allocated;
    bool quotes_collected;
    bool quotes_converted;
    bool quasiquotes_converted;
    bool inline_primitives_converted;
    bool constant_folded;
    bool constant_propagated;
    bool macros_expanded;
  };

PrimitiveCall make_primitive_call(std::vector<token>& tokens);
Quote make_quasiquote(std::vector<token>& tokens);
If make_if(std::vector<token>& tokens);
Literal make_literal(std::vector<token>& tokens);
Lambda make_lambda(std::vector<token>& tokens);
Let make_let(std::vector<token>& tokens, binding_type t);
Variable make_variable(std::vector<token>& tokens);
Expression make_expression(std::vector<token>& tokens);
Begin make_begin(std::vector<token>& tokens, bool implicit);
Set make_set(std::vector<token>& tokens);
FunCall make_fun(std::vector<token>& tokens);
Quote make_quote(std::vector<token>& tokens);
Quote make_unquote(std::vector<token>& tokens);
Quote make_unquote_splicing(std::vector<token>& tokens);
Cond make_cond(std::vector<token>& tokens);
Case make_case(std::vector<token>& tokens);
Do make_do(std::vector<token>& tokens);
ForeignCall make_foreign_call(std::vector<token>& tokens);

SKIWI_SCHEME_API Program make_program(std::vector<token>& tokens);
SKIWI_END
