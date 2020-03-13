#pragma once


DefineClassS(FileDescriptor);

class FileDescriptor
{
  int mFd;

public:
  operator int() const { return mFd; }
  int Descriptor() const { return mFd; }

public:
  FileDescriptor(int _Fd): mFd(_Fd) { }
};
