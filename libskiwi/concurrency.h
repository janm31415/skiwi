#pragma once

#include <tbb/parallel_for.h>

#include "compiler_options.h"

SKIWI_BEGIN

template <class _Type, class TFunctor>
void parallel_for(_Type first, _Type last, TFunctor fun, const compiler_options& ops)
  {
  if (ops.parallel)
    {
    tbb::parallel_for(first, last, fun);
    }
  else
    {
    for (_Type i = first; i != last; ++i)
      fun(i);
    }
  }

SKIWI_END
