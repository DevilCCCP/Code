#pragma once

#include <QtGlobal>


class H264NalUnit
{
  const char* mData;
  int         mSize;

  int         mPosUnit;
  int         mPosEnd;
  int         mPosNextUnit;

public:
  int CurrentUnitPos() { return mPosUnit; }
  int CurrentUnitEnd() { return mPosEnd; }
  const char* CurrentUnit() { return mData + mPosUnit; }
  int CurrentUnitSize() { return mPosEnd - mPosUnit; }
  const char* CurrentMarkedUnit() { return mData + mPosUnit - 3; }
  int CurrentMarkedUnitSize() { return mPosEnd - (mPosUnit - 3); }

public:
  static int Seek(const char* data);
  static int Seek(const char* data, int size);

  bool FindNext();

public:
  H264NalUnit(const char* _Data, int _Size);
};
