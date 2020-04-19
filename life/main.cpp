#include <iostream>

#include "window.h"

int main(int argc, char** argv)
  {
  const int w = 800;
  const int h = 600;
  auto wh = create_window("title", w, h);

  auto wh2 = create_window("second window", w, h);

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

  uint32_t* color_im = new uint32_t[w*h];
  for (int y = 0; y < h; ++y)
    {
    uint32_t* p_im = color_im + y * w;
    for (int x = 0; x < w; ++x)
      {      
      uint32_t r = ((x / 20) % 2) | ((y / 20) % 2) ? 255 : 0;
      uint32_t g = ((y / 20) % 2) ? 255 : 0;
      uint32_t b = ((x / 20) % 2) ? 255 : 0;
      *p_im = 0xff000000 | (b << 16) | (g << 8) | r;
      ++p_im;
      }
    }

  paint(wh2, (uint8_t*)color_im, w, h, -4);

  bool quit = false;
  while (!quit)
    {
    std::cout << "> ";    
    std::string input;
    std::getline(std::cin, input);
    if (input == "quit" || input == "exit")
      quit = true;
    if (input == "gray")
      paint(wh, (uint8_t*)im, w, h, 1);
    if (input == "color")
      paint(wh, (uint8_t*)color_im, w, h, 4);
    if (input == "small")
      resize(wh, 400, 300);
    if (input == "large")
      resize(wh, 1000, 750);
    }
  delete[] im;
  delete[] color_im;
  close_window(wh);
  close_window(wh2);
  return 0;
  }