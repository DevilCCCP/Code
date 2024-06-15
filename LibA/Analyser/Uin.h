#pragma once

#include <QVector>
#include <QMap>
#include <QList>
#include <QImage>

#include <Lib/Include/Common.h>

#include "Hyst.h"
#include "ByteRegion.h"
#include "Region.h"


class Analyser;

class Uin
{
  Analyser*                 mAnalyser;

  struct Color {
    int Black;
    int Gray;
    int White;
    Color(): Black(0), Gray(0), White(0) { }
  };
  struct CharInfo {
    QChar          Char;
    ByteRegion     Source;
    ByteRegion     Value;
    QVector<Color> HorzColor;
    QVector<Color> VertColor;
  };
  QList<CharInfo>           mCharInfoList;
  CharInfo*                 mCurrentCharInfo;

  ByteRegion*               mSourceData;
  int                       mQuality;
  QChar                     mChar;

  typedef QMap<QChar, qreal> CharMatchMap;
  struct TestInfo {
    ByteRegion  Value;
    int            WhiteMin;
    int            BlackMax;
    QVector<Color> HorzColor;
    QVector<Color> VertColor;
    CharMatchMap   CharMatch;
    qreal          BestMatch;
    QChar          BestChar;
  };
  QList<TestInfo>           mTestInfoList;
  TestInfo*                 mCurrentTestInfo;
  bool                      mNeedRecalc;

public:
  void AddChar(const QChar& _Char, const QImage& _Image);

  void BeginTest();
  void BeginChar(const ByteRegion& region, int whiteMin = 170, int blackMax = 85);
  void TrimChar(QRect* rect = nullptr);
  void TrimCharVert(int* left = nullptr, int* right = nullptr);
  void TrimCharHorz(int* top = nullptr, int* bottom = nullptr);
  void TestChar();
  void TestCharRadius(int radius = 0);

  qreal GetBestMatch();
  QChar GetBestChar();

  bool DumpBase(ByteRegion* debug);
  bool DumpTest(ByteRegion* debug);
  bool DumpDigits(ByteRegion* debug);

private:
  void Recalc();

  qreal MatchHyst(const CharInfo& charInfo, int xDisp, int yDisp);
  qreal MatchValue(const CharInfo& charInfo, int xDisp, int yDisp);

  static int ColorValue(const Uin::Color& a);
  static bool ColorComp(const Uin::Color& a, const Uin::Color& b);

public:
  Uin(Analyser* _Analyser);
  ~Uin();
};
