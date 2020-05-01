#pragma once

#include "namespace.h"
#include <stdint.h>
#include <vector>

SKIWI_BEGIN

struct liveness_range
  {
  uint64_t first, last; // interval containing start and end point of liveness of variable
  };

SKIWI_END
