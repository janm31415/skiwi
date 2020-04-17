#pragma once

#include <iostream>
#include <string>
#include <stdint.h>


class IWindowListener
  {
  public:
    virtual void OnClose() = 0;
    virtual void OnKeyDown(int) {};
  };

struct WindowHandleData;
typedef WindowHandleData* WindowHandle;

WindowHandle create_window(const std::string& id, const std::string& title, int w, int h, bool fullscreen = false, std::ostream* p_stream = nullptr);
WindowHandle create_window(const std::string& id, const std::string& title, int x, int y, int w, int h, bool fullscreen = false, std::ostream* p_stream = nullptr);

void close_window(WindowHandle& h_wnd);

// if h is negative the image will be flipped upside down
// if channels is negative the order of the colors will be flipped (e.g. convert rgb to bgr)
void paint(WindowHandle h_wnd, const uint8_t* bytes, int w, int h, int channels);

void resize(WindowHandle h_wnd, int w, int h);

void register_listener(WindowHandle h_wnd, IWindowListener* listener);
