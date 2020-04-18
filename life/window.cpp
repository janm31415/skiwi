#include "window.h"
#include <memory>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <atomic>
#endif
#include <condition_variable>

struct WindowHandleData
  {
  ~WindowHandleData()
    {
#ifdef _WIN32
    if (w != 0 && h != 0)
      free(bytes);
#else
    if (im)
      XDestroyImage(im);
    if (display)
      XCloseDisplay(display);
#endif
    }
#ifdef _WIN32
  HWND h_wnd;
  uint8_t* bytes;
#else
  Display* display;
  Window win;
  Visual* visual;
  XImage* im;
  GC gc;
  std::atomic<bool> quit;
#endif
  std::unique_ptr<std::thread> t;
  int w, h, channels;
  int x, y;
  std::mutex mt;
  std::condition_variable cv;
  bool initialised;
  };

#ifdef _WIN32

namespace
  {
  LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    switch (msg)
      {
      case WM_ERASEBKGND:
        // Do not erase the background to avoid flickering
        // We'll redraw the full window anyway
        break;
      case WM_CLOSE:
      {
      DestroyWindow(hwnd);
      break;
      }
      case WM_SIZE:
      {
      InvalidateRect(hwnd, NULL, TRUE);
      break;
      }
      case WM_DESTROY:
        PostQuitMessage(0);
        break;
      case WM_PAINT:
      {
      WindowHandle wh = (WindowHandle)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (wh && wh->w && wh->h)
        {
        RECT rect;
        GetClientRect(hwnd, &rect);
        PAINTSTRUCT ps;
        HDC hdc;
        HDC hdcMem;
        HGDIOBJ oldBitmap;
        hdc = BeginPaint(hwnd, &ps);
        hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbm = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
        oldBitmap = SelectObject(hdcMem, hbm);

        BITMAPINFO* pbmi = (BITMAPINFO*)alloca(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
        ZeroMemory(pbmi, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        if (wh->channels == 1)
          {
          pbmi->bmiHeader.biBitCount = 8;
          for (int i = 0; i < 256; ++i)
            {
            pbmi->bmiColors[i].rgbRed = BYTE(i);
            pbmi->bmiColors[i].rgbGreen = BYTE(i);
            pbmi->bmiColors[i].rgbBlue = BYTE(i);
            pbmi->bmiColors[i].rgbReserved = 0;
            }
          }
        else if (wh->channels == 4)
          {
          pbmi->bmiHeader.biBitCount = 32;
          }
        else
          pbmi->bmiHeader.biBitCount = 24;
        pbmi->bmiHeader.biCompression = BI_RGB;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biWidth = wh->w;
        pbmi->bmiHeader.biHeight = -wh->h;
        wh->mt.lock();
        SetStretchBltMode(hdcMem, COLORONCOLOR);
        StretchDIBits(hdcMem,
          0, 0, rect.right - rect.left, rect.bottom - rect.top,
          0, 0, wh->w, wh->h,
          wh->bytes, pbmi, 0, SRCCOPY);
        wh->mt.unlock();
        BitBlt(hdc, 0, 0,
          rect.right - rect.left, rect.bottom - rect.top,
          hdcMem, 0, 0,
          SRCCOPY);
        DeleteObject(hbm);
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        }
      break;
      }
      default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
      }
    return 0;
    }

  HWND _create_window(const std::string& title, int x, int y, int w, int h)
    {
    HINSTANCE hInstance = GetModuleHandle(0);

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = _wnd_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "window";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
      {
      return NULL;
      }

    // Step 2: Creating the Window

    HWND hwnd;

    hwnd = CreateWindowEx(
      WS_EX_CLIENTEDGE,
      "window",
      title.c_str(),
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, w, h,
      NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
      {
      return NULL;
      }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
    }

  void _create_window_with_message_loop(HWND* h_wnd, WindowHandle user_data, const std::string& title, int x, int y, int w, int h)
    {
    user_data->mt.lock();
    *h_wnd = _create_window(title, x, y, w, h);
    SetWindowLongPtr(*h_wnd, GWLP_USERDATA, (LONG_PTR)user_data);
    user_data->initialised = true;
    user_data->cv.notify_all();
    user_data->mt.unlock();

    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
      {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
      }
    }

  std::unique_ptr<std::thread> _create_threaded_window(HWND* h_wnd, WindowHandle user_data, const std::string& title, int x, int y, int w, int h)
    {
    user_data->initialised = false;
    std::unique_ptr<std::thread> res(new std::thread(_create_window_with_message_loop, h_wnd, user_data, title, x, y, w, h));
    std::unique_lock<std::mutex> lk(user_data->mt);
    if (!user_data->initialised)
      user_data->cv.wait(lk, [&] {return user_data->initialised; });
    return res;
    }
  }

void resize(WindowHandle h_wnd, int w, int h)
  {
  RECT rect;
  GetWindowRect(h_wnd->h_wnd, &rect);
  SetWindowPos(h_wnd->h_wnd, NULL, rect.left, rect.top, w, h, NULL);
  InvalidateRect(h_wnd->h_wnd, NULL, TRUE);
  }

#else

namespace
  {
  
  static bool XThreads_initialized = false;

  GC create_gc(Display* display, Window win, int reverse_video)
    {
    GC gc;
    XGCValues values;
    int screen_num = DefaultScreen(display);
    unsigned long valuemask = 0;

    gc = XCreateGC(display, win, valuemask, &values);

    if (reverse_video)
      {
      XSetForeground(display, gc, WhitePixel(display, screen_num));
      XSetBackground(display, gc, BlackPixel(display, screen_num));
      }
    else
      {
      XSetForeground(display, gc, BlackPixel(display, screen_num));
      XSetBackground(display, gc, WhitePixel(display, screen_num));
      }

    unsigned int line_width = 2;
    int line_style = LineSolid;
    int cap_style = CapButt;
    int join_style = JoinBevel;

    XSetLineAttributes(display, gc, line_width, line_style, cap_style, join_style);
    XSetFillStyle(display, gc, FillSolid);

    return gc;
    }

  void _create_window_with_message_loop(WindowHandle user_data, const std::string& title, int x, int y, int w, int h)
    {
    user_data->mt.lock();
    user_data->display = XOpenDisplay(0);
    int screen_num = DefaultScreen(user_data->display);

    user_data->win = XCreateSimpleWindow(user_data->display, RootWindow(user_data->display, screen_num),
      x, y, w, h, 2, BlackPixel(user_data->display, screen_num),
      WhitePixel(user_data->display, screen_num));

    user_data->visual = DefaultVisual(user_data->display, screen_num);
    user_data->gc = create_gc(user_data->display, user_data->win, 0);

    XMapWindow(user_data->display, user_data->win);
    XSync(user_data->display, False);
    XSelectInput(user_data->display, user_data->win, ExposureMask | StructureNotifyMask);

    Atom wmDeleteMessage = XInternAtom(user_data->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(user_data->display, user_data->win, &wmDeleteMessage, 1);

    user_data->quit = false;
    user_data->initialised = true;
    user_data->cv.notify_all();
    user_data->mt.unlock();

    while (!user_data->quit)
      {
      while (user_data->display && XPending(user_data->display))
        {
        XEvent ev;
        XNextEvent(user_data->display, &ev);
        //printf("event type: %d\n", ev.type); 
        switch (ev.type)
          {
          case Expose:
          {
          user_data->mt.lock();
          if (user_data->im)
            {
            XPutImage(user_data->display, user_data->win, user_data->gc, user_data->im, 0, 0, 0, 0, user_data->w, user_data->h);
            }
          user_data->mt.unlock();
          break;
          }
          case ClientMessage:
          {
          user_data->quit = true;
          XCloseDisplay(user_data->display);
          user_data->display = nullptr;
          break;
          }
          default: break;
          }
        }
      }
    }

  std::unique_ptr<std::thread> _create_threaded_window(WindowHandle user_data, const std::string& title, int x, int y, int w, int h)
    {
    user_data->initialised = false;
    std::unique_ptr<std::thread> res(new std::thread(_create_window_with_message_loop, user_data, title, x, y, w, h));
    std::unique_lock<std::mutex> lk(user_data->mt);
    if (!user_data->initialised)
      user_data->cv.wait(lk, [&] {return user_data->initialised; });
    return res;
    }
  }

void resize(WindowHandle h_wnd, int w, int h)
  {
  XResizeWindow(h_wnd->display, h_wnd->win, w, h);
  XFlush(h_wnd->display);
  }

#endif

void close_window(WindowHandle& h_wnd)
  {
  if (h_wnd)
    {
#ifdef _WIN32
    PostMessage(h_wnd->h_wnd, WM_CLOSE, NULL, NULL);
#else
    //XEvent event;
    //event.type = 
    //XSendEvent
    //XCloseDisplay(h_wnd->display);
    //XDestroyWindow(h_wnd->display, h_wnd->win);
    //XEvent ev;   
    //ev.type = DestroyNotify;
    //int res = XSendEvent(h_wnd->display, h_wnd->win, True, StructureNotifyMask, (XEvent*)&ev);
    //if (res != 0)
    //  printf("Send successfully\n");
    //else
    //  printf("sending failed\n");
    h_wnd->quit = true;
#endif
    h_wnd->t->join();
    delete h_wnd;
    h_wnd = nullptr;
    }
  }

WindowHandle create_window(const std::string& title, int x, int y, int w, int h)
  {
#ifdef _WIN32
  WindowHandle handle = new WindowHandleData();
  handle->h_wnd = nullptr;
  handle->bytes = nullptr;
#else
  if (!XThreads_initialized)
    {
    XInitThreads();
    XThreads_initialized = true;
    }
  WindowHandle handle = new WindowHandleData();
  handle->display = nullptr;
  handle->im = nullptr;
#endif
  handle->w = 0;
  handle->h = 0;
  handle->x = x;
  handle->y = y;
  handle->channels = 0;
#ifdef _WIN32
  handle->t = _create_threaded_window(&(handle->h_wnd), handle, title, x, y, w, h);
#else
  handle->t = _create_threaded_window(handle, title, x, y, w, h);
#endif
  return handle;
  }

WindowHandle create_window(const std::string& title, int w, int h)
  {
  return create_window(title, 0, 0, w, h);
  }

namespace
  {
  void _process_pixels(uint8_t* dst, const uint8_t* src, int width, int height, int channels, bool bFlipColors, bool bFlip)
    {
    int widthInBytes = width * channels;
    int numBytes = widthInBytes * height;

    if (!bFlipColors)
      {
      if (bFlip)
        {
        for (int y = 0; y < height; ++y)
          {
          memcpy(dst + (y * widthInBytes), src + ((height - y - 1) * widthInBytes), widthInBytes);
          }
        }
      else {
        memcpy(dst, src, numBytes);
        }
      }
    else {
      if (bFlip)
        {

        int x = 0;
        int y = (height - 1) * widthInBytes;
        src += y;

        for (int i = 0; i < numBytes; i += channels)
          {
          if (x >= width)
            {
            x = 0;
            src -= widthInBytes * 2;
            }

          for (int j = channels - 1; j >= 0; --j)
            {
            *dst = *(src + j);
            ++dst;
            }

          src += channels;
          ++x;
          }
        }
      else {
        for (int i = 0; i < numBytes; i += channels)
          {
          for (int j = channels - 1; j >= 0; --j)
            {
            *dst = *(src + j);
            ++dst;
            }
          src += channels;
          }
        }
      }
    }

  void _process_pixels_32(uint32_t* dst, const uint8_t* src, int width, int height, int channels, bool bFlipColors, bool bFlip)
    {
    if (channels == 4)
      return _process_pixels((uint8_t*)dst, src, width, height, channels, bFlipColors, bFlip);
    else if (channels == 1)
      {
      for (int y = 0; y < height; ++y)
        {
        uint32_t* p_dst = bFlip ? dst + (height - y - 1)*width : dst + y * width;
        const uint8_t* p_src = src + y * width;
        for (int x = 0; x < width; ++x)
          {
          uint32_t clr = 0xff000000 | ((uint32_t)*p_src << 16) | ((uint32_t)*p_src << 8) | ((uint32_t)*p_src);
          *p_dst = clr;
          ++p_dst;
          ++p_src;
          }
        }
      }
    else
      throw std::runtime_error("channels other than 1 or 4 are not supported on Linux");
    }

  } // namespace




void paint(WindowHandle h_wnd, const uint8_t* bytes, int w, int h, int channels)
  {
  bool flip_topdown = false;
  if (h < 0)
    {
    flip_topdown = true;
    h = -h;
    }
  bool flip_colors = false;
  if (channels < 0)
    {
    flip_colors = true;
    channels = -channels;
    }
#ifdef _WIN32
  h_wnd->mt.lock();
  if (h_wnd->w != w || h_wnd->h != h || h_wnd->channels != channels)
    {
    if (h_wnd->channels == 0)
      {
      h_wnd->bytes = (uint8_t*)malloc(sizeof(uint8_t)*w*h*channels);
      }
    else
      {
      h_wnd->bytes = (uint8_t*)realloc(h_wnd->bytes, sizeof(uint8_t)*w*h*channels);
      }
    h_wnd->w = w;
    h_wnd->h = h;
    h_wnd->channels = channels;
    }
  _process_pixels(h_wnd->bytes, bytes, w, h, channels, flip_colors, flip_topdown);
  h_wnd->mt.unlock();
  InvalidateRect(h_wnd->h_wnd, NULL, TRUE);
#else
  h_wnd->mt.lock();
  if (h_wnd->im)
    XDestroyImage(h_wnd->im);

  char* data;
  data = (char*)malloc(sizeof(uint8_t)*w*h * 4);

  h_wnd->w = w;
  h_wnd->h = h;
  h_wnd->channels = channels;

  _process_pixels_32((uint32_t*)data, bytes, w, h, channels, flip_colors, flip_topdown);

  int screen_num = DefaultScreen(h_wnd->display);

  h_wnd->im = XCreateImage(h_wnd->display, h_wnd->visual, DefaultDepth(h_wnd->display, screen_num), ZPixmap, 0, data, w, h, 32, 0);
  
  XFlush(h_wnd->display);
  
  XEvent ev;   
  ev.type = Expose;
  int res = XSendEvent(h_wnd->display, h_wnd->win, True, ExposureMask, (XEvent*)&ev);
  
  h_wnd->mt.unlock();
#endif
  }