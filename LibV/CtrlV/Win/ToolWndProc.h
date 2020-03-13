#pragma once

#include <Windows.h>


const int kToolButtonWidth = 76;
const int kToolButtonImageWidth = 64;
const int kToolButtonCount = 7;
const int kToolButtonMargin = 10;
const int kToolMargin = 80;
const int kToolLong = kToolButtonWidth * kToolButtonCount + (kToolButtonCount + 1) * kToolButtonMargin;
const int kToolWidth = kToolButtonWidth + 2 * kToolButtonMargin;

const int kExitButtonId = 1001;
const int kLayout0ButtonId = 1002;
const int kLayout1ButtonId = 1003;
const int kLayout2ButtonId = 1004;
const int kLayout3ButtonId = 1005;
const int kDesktopOnlyButtonId = 1006;
const int kDesktopNoneButtonId = 1007;

const int kToolsStartHideMs = 2000;
const int kToolsFullHideMs = 5000;
const int kToolsMaxAlpha = 180;
const int kToolsBorder = 2;

HWND CreateToolWnd(const QRect& screenRect, HWND mainWnd, LPVOID parent);
