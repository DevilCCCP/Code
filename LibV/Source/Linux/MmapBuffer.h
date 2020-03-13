#pragma once

#include <Lib/Include/Common.h>


DefineClassS(MmapBuffer);
DefineClassS(FileDescriptor);

class MmapBuffer
{
  FileDescriptorS mFileDescriptor;
  void*           mMemory;
  int             mSize;

public:
  void* data() const { return mMemory; }
  operator void*() const { return mMemory; }

public:
  MmapBuffer(const FileDescriptorS& _FileDescriptor, int size, off_t offset = 0);
  ~MmapBuffer();
};
