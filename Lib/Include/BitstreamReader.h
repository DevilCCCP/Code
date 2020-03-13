#pragma once


class BitstreamReader
{
  const char* mData;
  int         mSize;
  int         mPositionByte;
  int         mBitMask;

  bool        mEndOfData;

public:
  const char& CurrentByte() { return mData[mPositionByte]; }

  int Size() { return mSize; }
  int BytePosition() { return mPositionByte; }

  bool EndOfDataReached() { return mEndOfData; }
  bool EndOfData() { return mEndOfData = mPositionByte >= mSize; }
  bool ByteAligned() { return (mBitMask == (1 << 7)); }

  void SkipBits(int n)
  {
    if (n >= 8) {
      mPositionByte += (n >> 3);
      n &= 0x7;
    }
    while (n-- > 0) {
      SkipBit();
    }
  }

  void SkipBytes(int n)
  {
    mPositionByte += n;
  }

  int U(int n)
  {
    if (EndOfData()) {
      return 0;
    }

    int result = 0;
    for (int i = 0; i < n; i++) {
      result <<= 1;
      int bit;
      if (!NextBit(bit)) {
        mEndOfData = true;
        return 0;
      }
      result += bit;
    }

    return result;
  }

  int Uev()
  {
    return Ev(false);
  }

  int Sev()
  {
    return Ev(true);
  }

private:
  int Ev(bool _signed)
  {
    if (EndOfData()) {
      return 0;
    }

    int bitCount = 0;

    int bit;
    while (true) {
      if (!NextBit(bit)) {
        return 0;
      } else if (bit) {
        break;
      }
      bitCount++;
    }

    int result = 1;
    for (int i = 0; i < bitCount; i++) {
      if (!NextBit(bit)) {
        return 0;
      }
      result <<= 1;
      result += bit;
    }

    result--;
    if (_signed) {
      result = (result + 1) / 2 * ((result % 2) == 0 ? -1 : 1);
    }
    return result;
  }

private:
  bool NextBit(int& bit)
  {
    bit = ((mData[mPositionByte] & mBitMask) != 0)? 1: 0;
    return SkipBit();
  }

  bool SkipBit()
  {
    if (mPositionByte >= mSize) {
      return false;
    }
    mBitMask >>= 1;
    if (mBitMask == 0) {
      mPositionByte++;
      mBitMask = (1 << 7);
    }
    return true;
  }

public:
  BitstreamReader(const char* _Data, int _Size, int _Position = 0)
    : mData(_Data), mSize(_Size)
  {
    mPositionByte = (_Position >> 3);
    mBitMask = 1 << (7 - (_Position & 7));
    mEndOfData = mSize <= 0;
  }

  ~BitstreamReader()
  { }
};
