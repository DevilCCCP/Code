#pragma once

#include <windows.h>
#include <QRect>


HWND CreateBackWnd(const QRect& screenRect, LPVOID parent);
