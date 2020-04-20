#pragma once

#include <stdint.h>
#include <vector>
#include <cassert>

/*
Conway's Game of life, as in Chapter 17 of Michael Abrash's Graphics Programming Black Book.

The rules of Conway's game of life are simple:
 - If a cell is on and has either two or three neighbors that are on in the current generation, it stays on; otherwise, the cell turns off.
 - If a cell is off and has exactly three “on” neighbors in the current generation, it turns on; otherwise, it stays off. 
*/

struct cell_map
  {
  cell_map() : width(0), height(0) {}

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
    return cells[y*width + x] & (uint8_t)1;
    }

  int width;
  int height;
  std::vector<uint8_t> cells;
  };

inline cell_map next_generation(const cell_map& cm)
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