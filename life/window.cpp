#include "window.h"
#include <memory>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#else

#endif
#include <condition_variable>

struct WindowHandleData
  {
  ~WindowHandleData()
    {
    if (w != 0 && h != 0)
      free(bytes);
    if (listener)
      listener->OnClose();
    }
#ifdef _WIN32
  HWND h_wnd;
#else

#endif
  std::unique_ptr<std::thread> t;
  uint8_t* bytes;
  int w, h, channels;
  int x, y;
  std::string id;
  std::mutex mt;
  std::condition_variable cv;
  bool initialised;
  IWindowListener* listener;
  };

#ifdef _WIN32

namespace
  {
  LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    switch (msg)
      {
      case WM_KEYDOWN:
      {
      WindowHandle wh = (WindowHandle)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (wh && wh->listener)
        wh->listener->OnKeyDown((int)wParam);
      break;
      }
      case WM_ERASEBKGND:
        // Do not erase the background to avoid flickering
        // We'll redraw the full window anyway
        break;
      case WM_CLOSE:
      {
      WindowHandle wh = (WindowHandle)GetWindowLongPtr(hwnd, GWLP_USERDATA);
      if (wh && wh->listener)
        wh->listener->OnClose();
      DestroyWindow(hwnd);
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

  HWND _create_window(const std::string& id, const std::string& title, int x, int y, int w, int h, std::ostream* p_stream, bool fullscreen)
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
    wc.lpszClassName = id.c_str();
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
      {
      if (p_stream)
        *p_stream << "Cannot register the window" << std::endl;
      return NULL;
      }

    // Step 2: Creating the Window

    HWND hwnd;
    if (fullscreen)
      {
      hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        id.c_str(),
        title.c_str(),
        WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        x, y, w, h,
        NULL, NULL, hInstance, NULL);
      LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
      lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
      SetWindowLong(hwnd, GWL_STYLE, lStyle);
      LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
      lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
      SetWindowLong(hwnd, GWL_EXSTYLE, lExStyle);
      SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
      }
    else
      {
      hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        id.c_str(),
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, w, h,
        NULL, NULL, hInstance, NULL);
      }
    if (hwnd == NULL)
      {
      if (p_stream)
        *p_stream << "Window creation failed" << std::endl;
      return NULL;
      }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return hwnd;
    }

  void _create_window_with_message_loop(HWND* h_wnd, WindowHandle user_data, const std::string& id, const std::string& title, int x, int y, int w, int h, std::ostream* p_stream, bool fullscreen)
    {
    user_data->mt.lock();
    *h_wnd = _create_window(id, title, x, y, w, h, p_stream, fullscreen);
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
    if (p_stream)
      *p_stream << "Done" << std::endl;
    }

  std::unique_ptr<std::thread> _create_threaded_window(HWND* h_wnd, WindowHandle user_data, const std::string& id, const std::string& title, int x, int y, int w, int h, std::ostream* p_stream, bool fullscreen)
    {
    user_data->initialised = false;
    std::unique_ptr<std::thread> res(new std::thread(_create_window_with_message_loop, h_wnd, user_data, id, title, x, y, w, h, p_stream, fullscreen));
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
  }

#else

void resize(WindowHandle h_wnd, int w, int h)
  {

  }

#endif

void close_window(WindowHandle& h_wnd)
  {
  if (h_wnd)
    {
#ifdef _WIN32
    PostMessage(h_wnd->h_wnd, WM_CLOSE, NULL, NULL);
#else

#endif
    h_wnd->t->join();
    delete h_wnd;
    h_wnd = nullptr;
    }
  }

WindowHandle create_window(const std::string& id, const std::string& title, int x, int y, int w, int h, bool fullscreen, std::ostream* p_stream)
  {
  WindowHandle handle = new WindowHandleData();
  handle->listener = nullptr;
#ifdef _WIN32
  handle->h_wnd = nullptr;
#else

#endif
  handle->bytes = nullptr;
  handle->w = 0;
  handle->h = 0;
  handle->x = x;
  handle->y = y;
  handle->channels = 0;
  handle->id = id;
#ifdef _WIN32
  handle->t = _create_threaded_window(&(handle->h_wnd), handle, id, title, x, y, w, h, p_stream, fullscreen);
#else

#endif
  return handle;
  }

WindowHandle create_window(const std::string& id, const std::string& title, int w, int h, bool fullscreen, std::ostream* p_stream)
  {
  return create_window(id, title, 0, 0, w, h, fullscreen, p_stream);
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
  }

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
#ifdef _WIN32
  InvalidateRect(h_wnd->h_wnd, NULL, TRUE);
#else

#endif
  }

void register_listener(WindowHandle h_wnd, IWindowListener* listener)
  {
  h_wnd->listener = listener;
  }