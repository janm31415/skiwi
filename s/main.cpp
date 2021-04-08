#include <iostream>
#include <libskiwi/libskiwi.h>

int main(int argc, char** argv)
  {
  skiwi::skiwi_parameters pars;
  pars.heap_size = 64 * 1024 * 1024;
  skiwi::scheme_with_skiwi(nullptr, nullptr, pars);

  skiwi::skiwi_repl(argc, argv);

  skiwi::skiwi_quit();

  return 0;
  }