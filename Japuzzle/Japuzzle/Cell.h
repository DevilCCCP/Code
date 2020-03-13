#pragma once

#include <QVector>


typedef QVector<int> Line;

class Cell
{
  enum EMark {
    eMarkNone,
    eMarkYes,
    eMarkNo,
  };

  bool  mReal;
  EMark mMark;
  int   mMarkLevel;

  enum EStoreReal {
    eStoreRealNo   = 0,
    eStoreRealYes  = 1,
  };

  enum EStoreMark {
    eStoreMarkNone = 0,
    eStoreMarkYes  = 10,
    eStoreMarkNo   = 1,
    eStoreMarkYes1 = 5,
    eStoreMarkNo1  = 2,
    eStoreMarkYes2 = 6,
    eStoreMarkNo2  = 3,
    eStoreMarkYes3 = 7,
    eStoreMarkNo3  = 4,
  };

public:
  bool IsMarkYes() const { return mMark == eMarkYes; }
  bool IsMarkNo() const { return mMark == eMarkNo; }
  bool HasMark() const { return mMark != eMarkNone; }
  int MarkLevel() const { return mMarkLevel; }
  bool Real() const { return mReal; }

  void SetReal(bool value)
  {
    mReal = value;
  }

  void SetMark(int mark, int level)
  {
    if (HasMark() && MarkLevel() < level) {
      return;
    }

    mMark = (mark != 0)? (mark > 0? eMarkYes: eMarkNo): eMarkNone;
    mMarkLevel = (mark != 0)? level: 0;
  }

  int  GetMark() const
  {
    switch (mMark) {
    case eMarkYes: return 1;
    case eMarkNo: return -1;
    default: return 0;
    }
  }

public:
  bool Load(const char& b)
  {
    int lowPart = ((int)b) % 10;
    int hiPart = ((int)b) / 10;
    mReal = lowPart == 1;
    switch (hiPart) {
    case eStoreMarkNone: mMark = eMarkNone; mMarkLevel = 0; break;
    case eStoreMarkYes : mMark = eMarkYes; mMarkLevel = 0; break;
    case eStoreMarkNo  : mMark = eMarkNo; mMarkLevel = 0; break;
    case eStoreMarkYes1: mMark = eMarkYes; mMarkLevel = 1; break;
    case eStoreMarkNo1 : mMark = eMarkNo; mMarkLevel = 1; break;
    case eStoreMarkYes2: mMark = eMarkYes; mMarkLevel = 2; break;
    case eStoreMarkNo2 : mMark = eMarkNo; mMarkLevel = 2; break;
    case eStoreMarkYes3: mMark = eMarkYes; mMarkLevel = 3; break;
    case eStoreMarkNo3 : mMark = eMarkNo; mMarkLevel = 3; break;
    default: mMark = eMarkNone; mMarkLevel = 0; return false;
    }
    return true;
  }

  bool Save(char& b)
  {
    int lowPart = mReal? 1: 0;
    int hiPart;
    switch (mMark) {
    case eMarkNone:
      hiPart = eStoreMarkNone;
      break;
    case eMarkYes:
      switch (mMarkLevel) {
      case 0: hiPart = eStoreMarkYes; break;
      case 1: hiPart = eStoreMarkYes1; break;
      case 2: hiPart = eStoreMarkYes2; break;
      case 3: hiPart = eStoreMarkYes3; break;
      default: return false;
      }
      break;
    case eMarkNo:
      switch (mMarkLevel) {
      case 0: hiPart = eStoreMarkNo; break;
      case 1: hiPart = eStoreMarkNo1; break;
      case 2: hiPart = eStoreMarkNo2; break;
      case 3: hiPart = eStoreMarkNo3; break;
      default: return false;
      }
      break;
    default: return false;
    }
    b = (char)(uchar)(lowPart + hiPart * 10);
    return true;
  }

  void Clear()
  {
    mMark      = eMarkNone;
    mMarkLevel = 0;
  }

public:
  Cell(): mReal(false), mMark(eMarkNone), mMarkLevel(0) { }
};
