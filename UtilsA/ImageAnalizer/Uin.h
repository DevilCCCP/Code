#pragma once

#include <QVector>
#include <QMap>
#include <QList>
#include <QImage>

#include <Lib/Include/Common.h>
#include <LibV/Include/Hyst.h>
#include <LibV/Include/Region.h>


class ImgAnalizer;

class Uin
{
  ImgAnalizer*              mImgAnalizer;

  struct Color {
    int Black;
    int Gray;
    int White;
    Color(): Black(0), Gray(0), White(0) { }
  };
  struct CharInfo {
    QChar          Char;
    Region<uchar>  Value;
    QVector<Color> HorzColor;
    QVector<Color> VertColor;
  };
  QList<CharInfo>           mCharInfoList;
  CharInfo*                 mCurrentCharInfo;

  Region<uchar>*            mSourceData;
  int                       mQuality;
  QChar                     mChar;

  typedef QMap<QChar, qreal> CharMatchMap;
  struct TestInfo {
    Region<uchar>  Value;
    int            WhiteMin;
    int            BlackMax;
    QVector<Color> HorzColor;
    QVector<Color> VertColor;
    CharMatchMap   CharMatch;
  };
  QList<TestInfo>           mTestInfoList;
  TestInfo*                 mCurrentTestInfo;

public:
  void AddChar(const QChar& _Char, const QImage& _Image);

  void BeginTestChar();
  void TestChar(const Region<uchar>& region, int whiteMin, int blackMax);

  bool DumpBase(Region<uchar>* debug);
  bool DumpTest(Region<uchar>* debug);

private:
  qreal MatchHyst(const CharInfo& charInfo, int xDisp, int yDisp);
  qreal MatchValue(const CharInfo& charInfo, int xDisp, int yDisp);

public:
  Uin(ImgAnalizer* _ImgAnalizer);
  ~Uin();
};
