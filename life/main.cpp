#include <iostream>

#include "window.h"

#include <stdint.h>
#include <vector>
#include <cassert>
#include <random>
#include <thread>

struct cell_map
  {
  cell_map(int w, int h) : width(w), height(h)
    {
    cells.resize(w*h, 0);
    }

  void set_cell(int x, int y)
    {
    assert(cell_state(x, y) == 0);

    auto cell_ptr = cells.begin() + (y*width) + x;
    int xoleft, xoright, yoabove, yobelow;

    if (x == 0)
      xoleft = width - 1;
    else
      xoleft = -1;
    if (y == 0)
      yoabove = width*(height-1);
    else
      yoabove = -width;
    if (x == (width - 1))
      xoright = -(width - 1);
    else
      xoright = 1;
    if (y == (height - 1))
      yobelow = -width * (height - 1);
    else
      yobelow = width;

    *(cell_ptr) |= (uint8_t)0x01;
    *(cell_ptr + (yoabove + xoleft)) += 2;
    *(cell_ptr + yoabove) += 2;
    *(cell_ptr + (yoabove + xoright)) += 2;
    *(cell_ptr + xoleft) += 2;
    *(cell_ptr + xoright) += 2;
    *(cell_ptr + (yobelow + xoleft)) += 2;
    *(cell_ptr + yobelow) += 2;
    *(cell_ptr + (yobelow + xoright)) += 2;
    }

  void clear_cell(int x, int y)
    {
    assert(cell_state(x, y) == 1);

    auto cell_ptr = cells.begin() + (y*width) + x;
    int xoleft, xoright, yoabove, yobelow;

    if (x == 0)
      xoleft = width - 1;
    else
      xoleft = -1;
    if (y == 0)
      yoabove = width * (height - 1);
    else
      yoabove = -width;
    if (x == (width - 1))
      xoright = -(width - 1);
    else
      xoright = 1;
    if (y == (height - 1))
      yobelow = -width * (height - 1);
    else
      yobelow = width;

    *(cell_ptr) &= ~((uint8_t)0x01);
    *(cell_ptr + yoabove + xoleft) -= 2;
    *(cell_ptr + yoabove) -= 2;
    *(cell_ptr + yoabove + xoright) -= 2;
    *(cell_ptr + xoleft) -= 2;
    *(cell_ptr + xoright) -= 2;
    *(cell_ptr + yobelow + xoleft) -= 2;
    *(cell_ptr + yobelow) -= 2;
    *(cell_ptr + yobelow + xoright) -= 2;
    }

  int cell_state(int x, int y) const
    {
    return cells[y*width+x] & (uint8_t)1;
    }

  int width;
  int height;
  std::vector<uint8_t> cells;  
  };

cell_map next_generation(const cell_map& cm)
  {
  cell_map next(cm);  
  auto it = cm.cells.begin();
  for (int y = 0; y < cm.height; ++y)
    {
    int x = 0;
    auto it_row_end = it + cm.width;
    while (it != it_row_end)
      {
      if (*it)
        {
        uint8_t count = *it >> 1;
        if (*it & 1)
          {
          // Cell is on; turn it off if it doesn't have
          // 2 or 3 neighbors
          if ((count != 2) && (count != 3)) 
            next.clear_cell(x, y);
          }
        else
          {
          // Cell is off; turn it on if it has exactly 3 neighbors
          if (count == 3) 
            next.set_cell(x, y);
          }
        }
      ++it;
      ++x;
      }
    }
  return next;
  }

struct listener : public IWindowListener
  {
  listener() : quit(false) {}
  virtual void OnClose() { quit = true; };

  bool quit;
  };

int main()
  {
  std::random_device rd;
  std::mt19937 gen(rd());

  int width = 200;
  int height = 200;
  cell_map cm(width, height);  

  for (int i = 0; i < width*height / 2; ++i)
    {
    int x = gen() % width;
    int y = gen() % height;
    if (cm.cell_state(x, y) == 0)
      cm.set_cell(x, y);
    }

  int magnifier = 4;
  auto wh = create_window("Game of life", width*magnifier, height*magnifier);

  std::vector<uint8_t> image(width*height, 0);

  listener l;
  register_listener(wh, &l);

  int generation = 0;
  while (!l.quit)
    {    
    auto it = cm.cells.begin();
    auto it_end = cm.cells.end();
    auto im_it = image.begin();
    for (; it != it_end; ++it, ++im_it)
      {
      *im_it = (*it & 1) ? 0 : 255;
      }
    paint(wh, image.data(), width, height, 1);
    auto next = next_generation(cm);
    ++generation;
    cm = next;
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
    }

  close_window(wh);

  return 0;
  }