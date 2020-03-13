#include <sys/mman.h>

#include "MmapBuffer.h"
#include "FileDescriptor.h"


MmapBuffer::MmapBuffer(const FileDescriptorS& _FileDescriptor, int size, off_t offset)
  : mFileDescriptor(_FileDescriptor), mSize(size)
{
  mMemory = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, mFileDescriptor->Descriptor(), offset);
}

MmapBuffer::~MmapBuffer()
{
  if (mMemory) {
    munmap(mMemory, mSize);
  }
}
