#include <iostream>

#include "window.h"

int main(int argc, char** argv)
  {
  const int w = 800;
  const int h = 600;
  auto wh = create_window("title", w, h);

  uint8_t* im = new uint8_t[w*h];
  for (int y = 0; y < h; ++y)
    {
    uint8_t* p_im = im + y * w;
    for (int x = 0; x < w; ++x)
      {
      *p_im = ((x / 20) % 2 | (y / 20) % 2) ? 255 : 0;
      ++p_im;
      }
    }

  paint(wh, im, w, h, 1);

  bool quit = false;
  while (!quit)
    {
    std::cout << "> ";    
    std::string input;
    std::getline(std::cin, input);
    if (input == "quit" || input == "exit")
      quit = true;
    }

  delete[] im;
  close_window(wh);
  return 0;
  }