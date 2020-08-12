#pragma once

#include "namespace.h"
#include <string>
#include <vector>

SKIWI_BEGIN

struct cinput_data
  {
  enum cinput_type
    {
    cin_double,
    cin_int
    };
  std::vector<std::pair<std::string, cinput_type>> parameters;
  };

SKIWI_END