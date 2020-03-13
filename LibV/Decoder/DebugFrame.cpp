#include <Windows.h>

#include "DebugFrame.h"

static bool gWindow = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_CREATE: gWindow = true; break;
  case WM_DESTROY: gWindow = false; break;
  }
  return DefWindowProcA(hWnd, message, wParam, lParam);
}

static HWND CreateDebugWindow()
{
  const char gClassName[] = "DecoderTestWindow";

  WNDCLASSEXA wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_DBLCLKS;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = NULL;
  wcex.hIcon         = NULL;
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName	 = NULL;
  wcex.lpszClassName = gClassName;
  wcex.hIconSm       = NULL;

  RegisterClassExA(&wcex);

  HWND hwnd = CreateWindowA(gClassName, "debug", WS_OVERLAPPEDWINDOW
                            , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT
                            , nullptr, nullptr, nullptr, nullptr);

  ShowWindow(hwnd, SW_SHOW);
  return hwnd;
}

static void DebugDrawFrame(const char* data, int width, int height, int stride)
{
  static HWND hwnd = CreateDebugWindow();
  MSG msg;
  while (::PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }

  if (!gWindow) {
    return;
  }
  RECT rect;
  GetClientRect(hwnd, &rect);

  HDC hdc = GetDC(hwnd);

  SetStretchBltMode(hdc, COLORONCOLOR);

  BITMAPINFO bi;
  memset(&bi, 0, sizeof(bi));
  BITMAPINFOHEADER& bih = bi.bmiHeader;
  bih.biSize = sizeof(bih);
  bih.biWidth = width;
  bih.biHeight = height;
  bih.biPlanes = 1;
  bih.biBitCount = 24;
  bih.biCompression = BI_RGB;
  bih.biSizeImage = 0;
  static char buf[800 * 600 * 4] = { 0 };
  int strided = (width * 3 + 3) / 4 * 4;
  for (int j = 0; j < height; j++) {
    const char* d = data + j*stride;
    char* b = buf + j*strided;
    for (int i = 0; i < width; i++) {
      *b++ = d[2];
      *b++ = d[1];
      *b++ = d[0];
      d += 3;
    }
  }

  SetStretchBltMode(hdc, HALFTONE);
  StretchDIBits(hdc, rect.left, rect.bottom - 1, rect.right - rect.left, -(rect.bottom - rect.top)
                , 0, 0, width, height, buf, &bi, DIB_RGB_COLORS, SRCCOPY);

  ReleaseDC(hwnd, hdc);
}
