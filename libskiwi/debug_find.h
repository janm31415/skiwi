#pragma once

#include <asm/namespace.h>
#include "visitor.h"
#include "parse.h"

SKIWI_BEGIN

template <class TFind>
struct debug_find_visitor : public base_visitor<debug_find_visitor<TFind>>
  {
  debug_find_visitor(TFind i_pred) : pred(i_pred), found(false)
    {
    }

  bool found;
  TFind pred;

  virtual bool _previsit(Expression& e)
    {
    if (pred(e))
      found = true;

    return true;
    }
  };

template <class T, class TFind>
bool find(T& expr, TFind pred)
  {
  debug_find_visitor<TFind> dfv(pred);
  visitor<T, debug_find_visitor<TFind>>::visit(expr, &dfv);
  return dfv.found;
  }

SKIWI_END
