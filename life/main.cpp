#include <iostream>

#include "life.h"
#include "window.h"

#include <atomic>
#include <memory>
#include <random>
#include <thread>

#include <libskiwi/libskiwi.h>

int width = 100;
int height = 100;

cell_map grid;
std::vector<uint8_t> image;
WindowHandle wh;
std::unique_ptr<std::thread> game_thread;
std::atomic<bool> stop_game_thread;

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

void run_game_loop()
  {
  while (!stop_game_thread)
    {
    grid = next_generation(grid);
    paint_grid();
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(16));
    }
  }

void scm_run()
  {
  stop_game_thread = false;
  game_thread.reset(new std::thread(run_game_loop));
  }

void scm_stop()
  {
  if (game_thread)
    {
    stop_game_thread = true;
    game_thread->join();
    game_thread.reset(nullptr);
    }
  }

void scm_resize(int64_t w, int64_t h)
  {
  scm_stop();
  if (w > 0)
    width = (int)w;
  if (h > 0)
    height = (int)h;
  grid = cell_map(width, height);
  paint_grid();
  }

void scm_randomize()
  {
  scm_stop();
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
  scm_stop();
  grid = cell_map(width, height);
  paint_grid();
  }

void* register_functions(void*)
  {
  using namespace skiwi;
  register_external_primitive("resize", (void*)&scm_resize, skiwi_void, skiwi_int64, skiwi_int64, "(resize w h) resizes the Game of Life grid to w x h cells.");
  register_external_primitive("randomize", (void*)&scm_randomize, skiwi_void, "(randomize) fills the Game of Life grid with random cells.");
  register_external_primitive("clear", (void*)&scm_clear, skiwi_void, "(clear) clears the Game of Life grid.");
  register_external_primitive("run", (void*)&scm_run, skiwi_void, "(run) starts the Game of Life simulation.");
  register_external_primitive("stop", (void*)&scm_stop, skiwi_void, "(stop) stops the Game of Life simulation.");
  return nullptr;
  }

int main()
  {
  grid = cell_map(width, height);  
  wh = create_window("Game of life", 512, 512);
  skiwi::scheme_with_skiwi(&register_functions);
  skiwi::skiwi_repl();
  skiwi::skiwi_quit();
  scm_stop();
  close_window(wh);

  return 0;
  }