#include <QtGlobal>

#include "H264NalUnit.h"


int H264NalUnit::Seek(const char* data)
{
  const char* start = data;
  while (reinterpret_cast<qintptr>(data) & 3) {
    if (*data == 0 && *(data + 1) == 0 && *(data + 2) == 1) {
      return data - start;
    }
    data++;
  }

  const quint32* p = (const quint32*)data;
  for (; ; ) {
    quint32 x = *p++;
    if ((x & 0x00ffffff) == 0x00010000) { return (const char*)p - start - 4; }
    if ((x & 0xffffff00) == 0x01000000) { return (const char*)p - start - 3; }
    if ((x & 0xffff0000) == 0x00000000) {
      const char* v = (const char*)p;
      if (*v == 0x01) {
        return (const char*)p - start - 2;
      }
    }
    if ((x & 0x0000ffff) == 0x00000100) {
      const char* v = (const char*)p;
      if (*(v - 5) == 0x00) {
        return (const char*)p - start - 5;
      }
    }
  }
}

int H264NalUnit::Seek(const char* data, int size)
{
  const char* start = data;
  const char* end = data + size - 2;
  while ((reinterpret_cast<qintptr>(data) & 3) && data < end) {
    if (*data == 0 && *(data + 1) == 0 && *(data + 2) == 1) {
      return data - start;
    }
    data++;
  }

  const quint32* p = (const quint32*)data;
  const quint32* pe = (const quint32*)(end - (reinterpret_cast<qintptr>(end) & 3));
  for (; p < pe; ) {
    quint32 x = *p++;
    if ((x & 0x00ffffff) == 0x00010000) { return (const char*)p - start - 4; }
    if ((x & 0xffffff00) == 0x01000000) { return (const char*)p - start - 3; }
    if ((x & 0xffff0000) == 0x00000000) {
      const char* v = (const char*)p;
      if (v < end + 2 && *v == 0x01) {
        return (const char*)p - start - 2;
      }
    }
    if ((x & 0x0000ffff) == 0x00000100) {
      const char* v = (const char*)p;
      if (*(v - 5) == 0x00) {
        return (const char*)p - start - 5;
      }
    }
  }

  data = (const char*)p;
  while ((reinterpret_cast<qintptr>(data) & 3) && data < end) {
    if (*data == 0 && *(data + 1) == 0 && *(data + 2) == 1) {
      return data - start;
    }
    data++;
  }

  return -1;
}

bool H264NalUnit::FindNext()
{
  if (mPosUnit < 0) {
    mPosUnit = Seek(mData, mSize);
  } else {
    mPosUnit = mPosNextUnit;
  }
  if (mPosUnit < 0) {
    return false;
  }
  mPosUnit += 3;

  mPosNextUnit = Seek(mData + mPosUnit, mSize - mPosUnit);
  if (mPosNextUnit >= 0) {
    mPosNextUnit += mPosUnit;
    mPosEnd = mPosNextUnit;
    while (mPosEnd > 0 && mData[mPosEnd-1] == 0) {
      mPosEnd--;
    }
  } else {
    mPosEnd = mSize;
  }
  return true;
}


H264NalUnit::H264NalUnit(const char* _Data, int _Size)
  : mData(_Data), mSize(_Size)
  , mPosUnit(-1)
{
}
