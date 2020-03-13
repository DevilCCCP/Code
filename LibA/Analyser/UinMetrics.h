#pragma once


class UinMetrics
{
  int mBaseWidth;

public:
  void SetBaseWidth(int _BaseWidth) { mBaseWidth = _BaseWidth; }
  void SetBaseHeight(int _BaseHeight) { mBaseWidth = WidthForHeight(_BaseHeight); }
  bool IsValid() const { return mBaseWidth > 0; }

  int Width()       const { return mBaseWidth; }
  int Height()      const { return mBaseWidth * 30 / 144; }
  int DigitWidth()  const { return mBaseWidth * 15 / 144; }
  int DigitHeight() const { return mBaseWidth * 23 / 144; }
  int PackWidth()   const { return mBaseWidth * 48 / 144; }
  int DigitThick()  const { return mBaseWidth * 5 / 144; }
  int CharWidth()   const { return mBaseWidth * 14 / 144; }
  int CharHeight()  const { return mBaseWidth * 18 / 144; }
  int PrefixWidth() const { return mBaseWidth * 12 / 144; }
  int PrefixHeight()const { return mBaseWidth * 18 / 144; }
  int CharThick()   const { return mBaseWidth * 4 / 144; }
  int CharDowner()  const { return mBaseWidth * 6 / 144; }
  int RegionUpper() const { return mBaseWidth * 1 / 144; }
  int SymbolValue() const { return 180;/*mBaseWidth > 100? 40: mBaseWidth > 60? 80: 110;*/ }

  static int WidthForHeight(int height) { return height * 144 / 30; }
  static int WidthForCharHeight(int height) { return height * 144 / 18; }
  static int WidthForDigitHeight(int height) { return height * 144 / 23; }

public:
  UinMetrics()
    : mBaseWidth(0)
  { }
  UinMetrics(const UinMetrics& other)
    : mBaseWidth(other.mBaseWidth)
  { }
  UinMetrics(int _BaseWidth)
    : mBaseWidth(_BaseWidth)
  { }
};
