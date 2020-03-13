#pragma once
#include <memory>
#include <cstring>


class BitstreamWriter
{
  char*       mData;
  int         mSize;
  int         mPositionByte;
  int         mBitMask;

public:
  bool EndOfData() const { return mPositionByte >= mSize; }
  int Position() const { return mPositionByte; }

  bool SkipBits(int n)
  {
    if (n >= 8) {
      int bytes = (n << 3);
      if (mPositionByte + bytes > mSize) {
        bytes = mSize - mPositionByte;
        memset(mData, 0, bytes);
        return false;
      }
      memset(mData, 0, bytes);
      n &= 0x7;
    }
    while (n-- > 0) {
      if (!SkipBit()) {
        return false;
      }
    }
    return true;
  }

  void U(int n, int value)
  {
    if (EndOfData()) {
      return;
    }

    for (int i = n-1; i >= 0; i--) {
      int bit = ((value >> i) & 0x1);
      if (!NextBit(bit)) {
        return;
      }
    }
  }

  void Uev(int value)
  {
    return Ev(false, value);
  }

  void Sev(int value)
  {
    return Ev(true, value);
  }

private:
  void Ev(bool _signed, int value)
  {
    if (EndOfData()) {
      return;
    }

    if (_signed) {
      value = (value <= 0)? -2 * value: 2 * value - 1;
    }
    value++;
    int zeroCount = 0;
    int testBit = 2;
    for (int i = 1; i < 32; i++) {
      if (value & testBit) {
        zeroCount = i;
      }
      testBit <<= 1;
    }

    U(zeroCount, 0);
    U(zeroCount + 1, value);
  }

private:
  bool NextBit(const int& bit)
  {
    if (bit) {
      mData[mPositionByte] |= mBitMask;
    }
    return SkipBit();
  }

  bool SkipBit()
  {
    mBitMask >>= 1;
    if (mBitMask == 0) {
      mPositionByte++;
      mBitMask = (1 << 7);
      if (mPositionByte < mSize) {
        mData[mPositionByte] = 0;
        return true;
      }
      return false;
    }
    return true;
  }

public:
  BitstreamWriter(char* _Data, int _Size, int _Position = 0)
    : mData(_Data), mSize(_Size)
  {
    mPositionByte = (_Position >> 3);
    if (mSize > mPositionByte) {
      mData[mPositionByte] = 0;
    }
    mBitMask = 1 << (7 - (_Position & 7));
  }

  ~BitstreamWriter()
  { }
};
