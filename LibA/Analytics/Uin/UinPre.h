#pragma once

#include <QVector>
#include <QLinkedList>

#include <Lib/Include/Common.h>
#include <LibA/Analytics/Hyst.h>
#include <LibV/Include/Rect.h>
#include <LibV/Va/Region.h>


DefineClassS(UinPre);
DefineClassS(Uin);

class UinPre
{
  enum ELpTopDirection {
    eLeft,
    eTop,
    eRight
  };
  ELpTopDirection  mDirection;
  int              mHeight;
  int              mHeightL;
  int              mHeightR;
  int              mLength;
  int              mLeftTop;
  int              mRightTop;
  int              mLastTop;
  int              mLastValue;
  const uchar*     mSrcl;
  const uchar*     mSrc;
  int              mSrcSize;
  uchar*           mResultData;

  Hyst             mTopHyst;
  int              mTopThreshold;

  struct Obj {
    Rectangle Dimentions;
  };

  struct PreDigit {
    int   Start;
    int   Finish;
    int   Top;
    int   Bottom;
    int   Quality;
    QChar Char;
  };

  struct Front {
    enum EFlag {
      eFlagNone  = 0,
      ePreWhite  = 1 << 0,
      ePostWhite = 1 << 1
    };

    int  From;
    int  To;
    int  Flag;
    bool Ok;

    Front(int _From, int _To): From(_From), To(_To), Flag(0), Ok(false) { }
    Front(): From(-1), To(-1), Flag(0), Ok(false) { }
  };

  QList<Obj>            mObjsList;
  QLinkedList<Obj>      mCurObjsList;
  int                   mNextRect;
  Region<uchar>*        mSourceData;
  Region<uchar>         mPreData;

  Region<uchar>         mObjSource;
  Hyst                  mObjHyst;
  Region<uchar>         mObjValue;
  std::vector<Front>    mObjFront;
  std::vector<int>      mVertMass;
  std::vector<int>      mHorzMass;

  std::vector<int>      mDigitMass;
  int                   mDigitsStart;
  int                   mDigitsFinish;
  Region<uchar>         mDigitSource;
  Hyst                  mDigitSpaceHyst;
  int                   mSpaceMax;
  PreDigit              mTestDigit;
  QLinkedList<PreDigit> mDigits;
  UinS                  mUin;

  QString               mDebugText;
  int                   mDebugDigitPos;

public:
  static int MinHeight();
  const QString& DebugText() { return mDebugText; }

public:
  void Update();
  void Clear();
  void Clear1();
  void Clear2();
  void Calc(Region<uchar>* region);
  void CalcVector(const QVector<uchar>& line, QVector<uchar>& marks);
  void CalcLine(const uchar* src, uchar* marks, int size);
  void CalcStage1(Region<uchar>* region);
  void CalcStage2(const Rectangle& rect);

  void MergeLines(uchar* line, uchar* linep, int size);
  void MergeLinesBack(uchar* line, uchar* linep, int size);
  void SelectLineBegin();
  void SelectLine(int j, uchar* line, int size);
  void SelectLineEnd();
  void ResetRect();
  bool NextRect(Rectangle& rect);

  void DebugValue(Region<uchar>& regionValue);
  void DebugValue2(Region<uchar>& regionValue);

private:
  void inline ApplyTop();
  void inline Reset(int h);
  void inline Reset();

  void CalcBackFront();
  void CalcDigitMass();
  void CalcDigitSpaces();
  void CalcDigits();
  void FindNextDigit(int& pos);
  void CalcTestDigit();

  static void CalcRegionDiff(const Region<uchar>& regionSrc, Region<uchar>& regionDst);
  static void CalcRegionHyst(const Region<uchar>& region, Hyst& hyst);
  void CalcRegionHyst2(const Region<uchar>& region, Hyst& hyst);
  void Mk2Color(const Hyst& hyst, const Region<uchar>& regionSrc, Region<uchar>& regionDst);
  void Mk3Color(const Hyst& hyst, const Region<uchar>& regionSrc, Region<uchar>& regionDst);
  void DebugHyst(const Hyst& hyst, Region<uchar>& debugRegion);

  bool IsValidObj(const Obj& obj);

  static inline int Median(int a, int b, int c);

public:
  UinPre();

  friend class BlockObj;
};
