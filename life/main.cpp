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
std::atomic<double> game_sleep_time = 50.0;

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
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(game_sleep_time));
    }
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

void scm_run()
  {
  scm_stop();
  stop_game_thread = false;
  game_thread.reset(new std::thread(run_game_loop));
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

void scm_game_sleep(int64_t s)
  {
  game_sleep_time = (double)s;
  }

void scm_set_cell(int64_t x, int64_t y)
  {
  scm_stop();
  if (x >= 0 && y >= 0 && x < grid.width && y < grid.height)
    {
    if (grid.cell_state((int)x, (int)y) == 0)
      grid.set_cell((int)x, (int)y);
    }
  paint_grid();
  }

void scm_clear_cell(int64_t x, int64_t y)
  {
  scm_stop();
  if (x >= 0 && y >= 0 && x < grid.width && y < grid.height)
    {
    if (grid.cell_state((int)x, (int)y) != 0)
      grid.clear_cell((int)x, (int)y);
    }
  paint_grid();
  }

void scm_next()
  {
  scm_stop();
  grid = next_generation(grid);
  paint_grid();
  }

void scm_gun()
  {
  scm_stop();
  if (width < 38)
    width = 38;
  if (height < 11)
    height = 11;
  grid = cell_map(width, height);

  grid.set_cell(1, 5);
  grid.set_cell(2, 5);
  grid.set_cell(1, 6);
  grid.set_cell(2, 6);

  grid.set_cell(35, 3);
  grid.set_cell(36, 3);
  grid.set_cell(35, 4);
  grid.set_cell(36, 4);

  grid.set_cell(11, 5);
  grid.set_cell(11, 6);
  grid.set_cell(11, 7);
  grid.set_cell(12, 4);
  grid.set_cell(12, 8);
  grid.set_cell(13, 3);
  grid.set_cell(13, 9);
  grid.set_cell(14, 3);
  grid.set_cell(14, 9);
  grid.set_cell(15, 6);
  grid.set_cell(16, 4);
  grid.set_cell(16, 8);
  grid.set_cell(17, 5);
  grid.set_cell(17, 6);
  grid.set_cell(17, 7);
  grid.set_cell(18, 6);

  grid.set_cell(21, 3);
  grid.set_cell(21, 4);
  grid.set_cell(21, 5);
  grid.set_cell(22, 3);
  grid.set_cell(22, 4);
  grid.set_cell(22, 5);
  grid.set_cell(23, 2);
  grid.set_cell(23, 6);
  grid.set_cell(25, 1);
  grid.set_cell(25, 2);
  grid.set_cell(25, 6);
  grid.set_cell(25, 7);
  paint_grid();
  }


void scm_space_rake()
  {
  scm_stop();
  if (width < 26)
    width = 26;
  if (height < 23)
    height = 23;
  grid = cell_map(width, height);

  grid.set_cell(2, 17);
  grid.set_cell(2, 19);
  grid.set_cell(3, 20);
  grid.set_cell(4, 20);
  grid.set_cell(5, 17);
  grid.set_cell(5, 20);
  grid.set_cell(6, 18);
  grid.set_cell(6, 19);
  grid.set_cell(6, 20);


  grid.set_cell(19, 19);
  grid.set_cell(19, 17);
  grid.set_cell(20, 16);
  grid.set_cell(21, 16);
  grid.set_cell(22, 19);
  grid.set_cell(22, 16);
  grid.set_cell(23, 16);
  grid.set_cell(23, 17);
  grid.set_cell(23, 18);  

  grid.set_cell(19, 5);
  grid.set_cell(19, 3);
  grid.set_cell(20, 2);
  grid.set_cell(21, 2);
  grid.set_cell(22, 5);
  grid.set_cell(22, 2);
  grid.set_cell(23, 2);
  grid.set_cell(23, 3);
  grid.set_cell(23, 4);

  grid.set_cell(8, 9);
  grid.set_cell(9, 8);
  grid.set_cell(9, 10);
  grid.set_cell(10, 7);
  grid.set_cell(10, 8);
  grid.set_cell(10, 10);
  grid.set_cell(10, 11);
  grid.set_cell(11, 10);
  grid.set_cell(11, 11);
  grid.set_cell(12, 10);
  grid.set_cell(12, 11);
  grid.set_cell(13, 10);
  grid.set_cell(13, 11);
  grid.set_cell(13, 12);

  grid.set_cell(11, 3);
  grid.set_cell(11, 4);
  grid.set_cell(12, 3);
  grid.set_cell(12, 4);
  grid.set_cell(12, 5);
  grid.set_cell(13, 2);
  grid.set_cell(13, 4);
  grid.set_cell(13, 5);
  grid.set_cell(14, 2);
  grid.set_cell(14, 3);
  grid.set_cell(14, 4);
  grid.set_cell(15, 3);

  grid.set_cell(17, 11);
  grid.set_cell(18, 9);
  grid.set_cell(18, 10);
  grid.set_cell(18, 11);
  grid.set_cell(18, 12);
  grid.set_cell(19, 8);
  grid.set_cell(19, 12);
  grid.set_cell(20, 8);
  grid.set_cell(20, 11);
  grid.set_cell(21, 9);
  grid.set_cell(21, 10);
  grid.set_cell(21, 11);

  paint_grid();
  }


void scm_spaceship()
  {
  scm_stop();
  if (width < 7)
    width = 7;
  if (height < 6)
    height = 6;
  grid = cell_map(width, height);

  grid.set_cell(2, 5);
  grid.set_cell(2, 3);
  grid.set_cell(3, 2);
  grid.set_cell(4, 2);
  grid.set_cell(5, 5);
  grid.set_cell(5, 2);
  grid.set_cell(6, 2);
  grid.set_cell(6, 3);
  grid.set_cell(6, 4);

  paint_grid();
  }

void* register_functions(void*)
  {
  using namespace skiwi;
  register_external_primitive("resize", (void*)&scm_resize, skiwi_void, skiwi_int64, skiwi_int64, "(resize w h) resizes the Game of Life grid to w x h cells.");
  register_external_primitive("randomize", (void*)&scm_randomize, skiwi_void, "(randomize) fills the Game of Life grid with random cells.");
  register_external_primitive("clear", (void*)&scm_clear, skiwi_void, "(clear) clears the Game of Life grid.");
  register_external_primitive("next", (void*)&scm_next, skiwi_void, "(next) shows the next generation of the Game of Life grid.");
  register_external_primitive("run", (void*)&scm_run, skiwi_void, "(run) starts the Game of Life simulation.");
  register_external_primitive("stop", (void*)&scm_stop, skiwi_void, "(stop) stops the Game of Life simulation.");
  register_external_primitive("game-sleep", (void*)&scm_game_sleep, skiwi_void, skiwi_int64, "(game-sleep <number>) waits <number> milliseconds between generations.");
  register_external_primitive("set-cell", (void*)&scm_set_cell, skiwi_void, skiwi_int64, skiwi_int64, "(set-cell <x> <y>) sets cell (x,y) on");
  register_external_primitive("clear-cell", (void*)&scm_clear_cell, skiwi_void, skiwi_int64, skiwi_int64, "(clear-cell <x> <y>) sets cell (x,y) off");
  register_external_primitive("gun", (void*)&scm_gun, skiwi_void, "(gun) generates the Gosper glider gun");
  register_external_primitive("space-rake", (void*)&scm_space_rake, skiwi_void, "(space-rake) generates the space-rake");
  register_external_primitive("spaceship", (void*)&scm_spaceship, skiwi_void, "(spaceship) generates the spaceship or glider");
  return nullptr;
  }

int main()
  {  
  wh = create_window("Game of life", 512, 512); // create window for visualization
  scm_randomize(); // fill grid cells at random  
  skiwi::scheme_with_skiwi(&register_functions); // start scheme compiler skiwi
  skiwi::skiwi_repl(); // start the skiwi repl
  skiwi::skiwi_quit(); // clean up skiwi
  scm_stop(); // stop the game thread
  close_window(wh); // close the visualization window

  return 0;
  }