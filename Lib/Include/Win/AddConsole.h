#pragma once

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#ifndef _USE_OLD_IOSTREAMS

using namespace std;

#endif

// maximum mumber of lines the output console should have

const WORD kMaxLines = 500;

inline void RedirectIo(FILE* fileCon, DWORD handle, char* mode)
{
  long h = (long)GetStdHandle(handle);
  int conHandle = _open_osfhandle(h, _O_TEXT);

  FILE *fileH = _fdopen(conHandle, mode);
  *fileCon = *fileH;
  setvbuf(fileCon, NULL, _IONBF, 0);
}

inline void AddConsole()
{
  AllocConsole();

  // set the screen buffer to be big enough to let us scroll text
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
  coninfo.dwSize.Y = kMaxLines;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

  RedirectIo(stdout, STD_OUTPUT_HANDLE, "w");
  RedirectIo(stdin, STD_INPUT_HANDLE, "r");
  RedirectIo(stderr, STD_ERROR_HANDLE, "w");
  ios::sync_with_stdio();
}
