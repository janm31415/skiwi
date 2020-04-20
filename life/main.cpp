#include <iostream>

#include "life.h"
#include "window.h"

#include <random>
#include <thread>

#include <libskiwi/libskiwi.h>

int width = 100;
int height = 100;

cell_map grid;
std::vector<uint8_t> image;
WindowHandle wh;

void paint_grid()
  {
  const int nr_bytes = grid.width*grid.height;
  if (image.size() != nr_bytes)
    image.resize(nr_bytes);
  auto it = grid.cells.begin();
  auto it_end = grid.cells.end();
  auto im_it = image.begin();
  while (it != it_end)    
    *im_it++ = (*it++ & 1) ? 0 : 255;    
  paint(wh, image.data(), grid.width, grid.height, 1);
  }

void scm_resize(int64_t w, int64_t h)
  {
  if (w > 0)
    width = (int)w;
  if (h > 0)
    height = (int)h;
  grid = cell_map(width, height);
  paint_grid();
  }

void scm_randomize()
  {
  std::random_device rd;
  std::mt19937 gen(rd());

  grid = cell_map(width, height);

  for (int i = 0; i < width*height / 2; ++i)
    {
    int x = gen() % width;
    int y = gen() % height;
    if (grid.cell_state(x, y) == 0)
      grid.set_cell(x, y);
    }
  paint_grid();
  }

void scm_clear()
  {
  grid = cell_map(width, height);
  paint_grid();
  }

void* register_functions(void*)
  {
  using namespace skiwi;
  register_external_primitive("resize", (void*)&scm_resize, skiwi_void, skiwi_int64, skiwi_int64, "(resize w h) resizes the Game of Life grid to w x h cells.");
  register_external_primitive("randomize", (void*)&scm_randomize, skiwi_void, "(randomize) fills the Game of Life grid with random cells.");
  register_external_primitive("clear", (void*)&scm_clear, skiwi_void, "(clear) clears the Game of Life grid.");
  return nullptr;
  }

int main()
  {
  grid = cell_map(width, height);  
  wh = create_window("Game of life", 512, 512);
  skiwi::scheme_with_skiwi(&register_functions);
  skiwi::skiwi_repl();
  skiwi::skiwi_quit();
  close_window(wh);

  /*
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
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(50));
    }

  close_window(wh);
  */
  return 0;
  }