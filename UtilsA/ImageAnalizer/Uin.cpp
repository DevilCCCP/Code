#include <QColor>
#include <QDebug>

#include "Uin.h"


const int kValueWhite = 250;
const int kValueBlack = 50;

void Uin::AddChar(const QChar& _Char, const QImage& _Image)
{
  mCharInfoList.append(CharInfo());
  mCurrentCharInfo = &mCharInfoList.last();
  mCurrentCharInfo->Char = _Char;
  int width = _Image.width();
  int height = _Image.height();
  mCurrentCharInfo->Value.SetSize(width, height);
  mCurrentCharInfo->HorzColor.resize(height);
  mCurrentCharInfo->VertColor.resize(width);

  for (int j = 0; j < height; j++) {
    const QRgb* data = reinterpret_cast<const QRgb*>(_Image.scanLine(j));
    uchar* dst = mCurrentCharInfo->Value.Line(j);
    Color* horzValue = &mCurrentCharInfo->HorzColor[j];
    Color* vertValue = &mCurrentCharInfo->VertColor[0];
    for (int i = 0; i < width; i++) {
      int v = QColor::fromRgb(*data).value();
      *dst = v;
      if (v >= kValueWhite) {
        horzValue->White++;
        vertValue->White++;
      } else if (v <= kValueBlack) {
        horzValue->Black++;
        vertValue->Black++;
      } else {
        horzValue->Gray++;
        vertValue->Gray++;
      }

      data++;
      dst++;
      vertValue++;
    }
  }
}

void Uin::BeginTestChar()
{
  mTestInfoList.clear();
}

void Uin::TestChar(const Region<uchar>& region, int whiteMin, int blackMax)
{
  mTestInfoList.append(TestInfo());
  mCurrentTestInfo = &mTestInfoList.back();

  mCurrentTestInfo->Value.SetSource(region);
  mCurrentTestInfo->WhiteMin = whiteMin;
  mCurrentTestInfo->BlackMax = blackMax;
  mCurrentTestInfo->HorzColor.resize(region.Height());
  mCurrentTestInfo->VertColor.resize(region.Width());

  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    Color* horzValue = &mCurrentTestInfo->HorzColor[j];
    Color* vertValue = &mCurrentTestInfo->VertColor[0];

    for (int i = 0; i < region.Width(); i++) {
      if (*src > whiteMin) {
        horzValue->White++;
        vertValue->White++;
      } else if (*src <= blackMax) {
        horzValue->Black++;
        vertValue->Black++;
      } else {
        horzValue->Gray++;
        vertValue->Gray++;
      }

      src++;
      vertValue++;
    }
  }

  foreach (const CharInfo& charInfo, mCharInfoList) {
    qreal bestMatch = 0;
    int bestI = 0;
    int bestJ = 0;
    for (int j = -2; j <= 2; j++) {
      for (int i = -4; i <= 4; i++) {
        qreal match = MatchHyst(charInfo, i, j);
        if (match > bestMatch) {
          bestI = i;
          bestJ = j;
          bestMatch = match;
        }
      }
    }
    if (bestMatch >= 1) {
      bestMatch = 0;
      for (int j = -1; j <= 1; j++) {
        for (int i = -1; i <= 1; i++) {
          qreal match = MatchValue(charInfo, i, j);
          if (match > bestMatch) {
//            bestI = i;
//            bestJ = j;
            bestMatch = match;
          }
        }
      }
    }
    mCurrentTestInfo->CharMatch.insert(charInfo.Char, bestMatch);
  }
}

bool Uin::DumpBase(Region<uchar>* debug)
{
  if (mCharInfoList.isEmpty()) {
    return false;
  }

  int n = 4;
  int m = (mCharInfoList.size() + 3) / 4;
  int width  = 0;
  int height = 0;
  for (int i = 0; i < mCharInfoList.size(); i++) {
    width  = qMax(width, 2*mCharInfoList.at(i).Value.Width());
    height = qMax(height, 2*mCharInfoList.at(i).Value.Height());
  }
  debug->SetSize(m * width, n * height);
  debug->ZeroData();

  m = 0;
  n = 0;
  for (int k = 0; k < mCharInfoList.size(); k++) {
    const CharInfo* info = &mCharInfoList.at(k);
    for (int j = 0; j < info->Value.Height(); j++) {
      const uchar* src = info->Value.Line(j);
      uchar* dst = debug->Data(m * width, n * height + j);
      for (int i = 0; i < info->Value.Width(); i++) {
        memcpy(dst, src, info->Value.Width());
      }
    }
    for (int j = 0; j < info->Value.Height(); j++) {
      const Color& color = info->HorzColor.at(j);
      uchar* dbg = debug->Data(m * width + info->Value.Width(), n * height + j);
      int denum = color.Black + color.White + color.Gray;
      int p1 = color.Black * info->Value.Width() / denum;
      int p2 = (color.Black + color.Gray) * info->Value.Width() / denum;
      int p3 = info->Value.Width();
      int i = 0;
      for (; i < p1; i++) {
        *dbg = 0;
        dbg++;
      }
      for (; i < p2; i++) {
        *dbg = 127;
        dbg++;
      }
      for (; i < p3; i++) {
        *dbg = 255;
        dbg++;
      }
    }
    for (int i = 0; i < info->Value.Width(); i++) {
      const Color& color = info->VertColor.at(i);
      int denum = color.Black + color.White + color.Gray;
      int p1 = color.Black * info->Value.Height() / denum;
      int p2 = (color.Black + color.Gray) * info->Value.Height() / denum;
      int p3 = info->Value.Height();
      int j = 0;
      for (; j < p1; j++) {
        *debug->Data(m * width + i, n * height + info->Value.Height() + j) = 0;
      }
      for (; j < p2; j++) {
        *debug->Data(m * width + i, n * height + info->Value.Height() + j) = 127;
      }
      for (; j < p3; j++) {
        *debug->Data(m * width + i, n * height + info->Value.Height() + j) = 255;
      }
    }

    if (++n >= 4) {
      n = 0;
      ++m;
    }
  }

  return true;
}

bool Uin::DumpTest(Region<uchar>* debug)
{
  int width = 0;
  int height = 0;
  foreach (const TestInfo& testInfo, mTestInfoList) {
    width  = qMax(width, 2*testInfo.Value.Width());
    height = qMax(height, 2*testInfo.Value.Height());
  }

  int n = 4;
  int m = (mTestInfoList.size() + 3) / 4;
  debug->SetSize(m * width, n * height);
  debug->ZeroData();
  for (int i = 0; i < mTestInfoList.size(); i++) {
    const TestInfo& info = mTestInfoList.at(i);
    m = i / 4;
    n = i % 4;
    QRect objRect(m * width, n * height, info.Value.Width(), info.Value.Height());

    for (int j = 0; j < objRect.height(); j++) {
      const uchar* src = info.Value.Line(j);
      uchar* dbg = debug->Data(objRect.left(), objRect.top() + j);

      for (int i = 0; i < objRect.width(); i++) {
        if (*src <= info.BlackMax) {
          *dbg = 0;
        } else if (*src < info.WhiteMin) {
          *dbg = 127;
        } else {
          *dbg = 255;
        }

        src++;
        dbg++;
      }
    }

    for (int j = 0; j < info.Value.Height(); j++) {
      const Color& color = info.HorzColor.at(j);
      uchar* dbg = debug->Data(m * width + info.Value.Width(), n * height + j);
      int denum = color.Black + color.White + color.Gray;
      int p1 = color.Black * info.Value.Width() / denum;
      int p2 = (color.Black + color.Gray) * info.Value.Width() / denum;
      int p3 = info.Value.Width();
      int i = 0;
      for (; i < p1; i++) {
        *dbg = 0;
        dbg++;
      }
      for (; i < p2; i++) {
        *dbg = 127;
        dbg++;
      }
      for (; i < p3; i++) {
        *dbg = 255;
        dbg++;
      }
    }
    for (int i = 0; i < info.Value.Width(); i++) {
      const Color& color = info.VertColor.at(i);
      int denum = color.Black + color.White + color.Gray;
      int p1 = color.Black * info.Value.Height() / denum;
      int p2 = (color.Black + color.Gray) * info.Value.Height() / denum;
      int p3 = info.Value.Height();
      int j = 0;
      for (; j < p1; j++) {
        *debug->Data(m * width + i, n * height + info.Value.Height() + j) = 0;
      }
      for (; j < p2; j++) {
        *debug->Data(m * width + i, n * height + info.Value.Height() + j) = 127;
      }
      for (; j < p3; j++) {
        *debug->Data(m * width + i, n * height + info.Value.Height() + j) = 255;
      }
    }

    QStringList matchText;
    QMap<qreal, QString> matchTextMap;
    for (auto itr = info.CharMatch.begin(); itr != info.CharMatch.end(); itr++) {
      matchTextMap.insert(-itr.value(), QString("%1: %2").arg(itr.key()).arg(itr.value()));
    }
    for (auto itr = matchTextMap.begin(); itr != matchTextMap.end(); itr++) {
      matchText << itr.value();
    }
    qDebug() << matchText.join("; ");
  }
  return true;
}

qreal Uin::MatchHyst(const CharInfo& charInfo, int xDisp, int yDisp)
{
  qreal k = (qreal)charInfo.Value.Height() / mCurrentTestInfo->Value.Height();
  qreal match = 0;

  const Color* horzColor = mCurrentTestInfo->HorzColor.constData();
  for (int j = 0; j < mCurrentTestInfo->Value.Height(); j++) {
    int y = (0.5 + j + yDisp) * k - 0.5;
    if (y < 0 || y >= charInfo.HorzColor.size()) {
      continue;
    }
    const Color& charColor = charInfo.HorzColor.at(y);
    qreal missB = qAbs(k * horzColor->Black - charColor.Black);
    qreal missW = qAbs(k * horzColor->White - charColor.White);
//    qreal missG = qAbs(k * horzColor->Gray - charColor.Gray);
    match += (1.0 - (missB + missW) / charInfo.Value.Width()) / mCurrentTestInfo->Value.Height();

    horzColor++;
  }

  const Color* vertColor = mCurrentTestInfo->VertColor.constData();
  for (int i = 0; i < mCurrentTestInfo->Value.Width(); i++) {
    int x = (0.5 + i + xDisp) * k - 0.5;
    if (x < 0 || x >= charInfo.VertColor.size()) {
      continue;
    }
    const Color& charColor = charInfo.VertColor.at(x);
    qreal missB = qAbs(k * vertColor->Black - charColor.Black);
    qreal missW = qAbs(k * vertColor->White - charColor.White);
//    qreal missG = qAbs(k * vertColor->Gray - charColor.Gray);
    match += (1.0 - (missB + missW) / charInfo.Value.Height()) / mCurrentTestInfo->Value.Width();

    vertColor++;
  }

  return match;
}

qreal Uin::MatchValue(const Uin::CharInfo& charInfo, int xDisp, int yDisp)
{
  qreal k = (qreal)charInfo.Value.Height() / mCurrentTestInfo->Value.Height();
  int match = 0;
  int count = 0;
  for (int j = 0; j < mCurrentTestInfo->Value.Height(); j++) {
    int y = (0.5 + j + yDisp) * k - 0.5;
    if (y < 0 || y >= charInfo.HorzColor.size()) {
      continue;
    }
    for (int i = 0; i < mCurrentTestInfo->Value.Width(); i++) {
      int x = (0.5 + i + xDisp) * k - 0.5;
      if (x < 0 || x >= charInfo.VertColor.size()) {
        continue;
      }
      const uchar* tst = mCurrentTestInfo->Value.Data(i, j);
      const uchar* chr = charInfo.Value.Data(x, y);
      int tstColor;
      if (*tst <= mCurrentTestInfo->BlackMax) { // black
        tstColor = -1;
      } else if (*tst < mCurrentTestInfo->WhiteMin) { // gray
        tstColor = 0;
      } else { // white
        tstColor = 1;
      }
      int chrColor;
      if (*chr <= kValueBlack) { // black
        chrColor = -1;
      } else if (*chr < kValueWhite) { // gray
        chrColor = 0;
      } else { // white
        chrColor = 1;
      }
      int chrDiff = 2 - qAbs(tstColor - chrColor);
      match += chrDiff;
      count++;
    }
  }
  return count > 0? (qreal)match / count: (qreal)0;
}


Uin::Uin(ImgAnalizer* _ImgAnalizer)
  : mImgAnalizer(_ImgAnalizer)
  , mSourceData(nullptr)
{
}

Uin::~Uin()
{
}

