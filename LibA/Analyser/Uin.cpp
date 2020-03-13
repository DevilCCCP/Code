#include <QColor>
#include <QDebug>

#include "Uin.h"


const int kValueWhite      = 250;
const int kValueBlack      = 50;
const int kMinRegionSize   = 6;
const qreal kMinDigitMatch = 1.5;

void Uin::AddChar(const QChar& _Char, const QImage& _Image)
{
  mCharInfoList.append(CharInfo());
  mCurrentCharInfo = &mCharInfoList.last();
  mCurrentCharInfo->Char = _Char;
  int width = _Image.width();
  int height = _Image.height();
  mCurrentCharInfo->Source.SetSize(width, height);
  mCurrentCharInfo->HorzColor.resize(height);
  mCurrentCharInfo->VertColor.resize(width);

  for (int j = 0; j < height; j++) {
    const QRgb* data = reinterpret_cast<const QRgb*>(_Image.scanLine(j));
    uchar* dst = mCurrentCharInfo->Source.Line(j);
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

  int maxVert = ColorValue(*std::max_element(mCurrentCharInfo->VertColor.begin(), mCurrentCharInfo->VertColor.end(), ColorComp));
  int maxHorz = ColorValue(*std::max_element(mCurrentCharInfo->HorzColor.begin(), mCurrentCharInfo->HorzColor.end(), ColorComp));
  int minVert = 2*maxVert/height + 1;
  int minHorz = 2*maxHorz/width + 1;
  int l = 0;
  for (; l < width; l++) {
    if (ColorValue(mCurrentCharInfo->VertColor.at(l)) >= minVert) {
      break;
    }
  }
  int r = width - 1;
  for (; r >= 0; r--) {
    if (ColorValue(mCurrentCharInfo->VertColor.at(r)) >= minVert) {
      break;
    }
  }
  int t = 0;
  for (; t < height; t++) {
    if (ColorValue(mCurrentCharInfo->HorzColor.at(t)) >= minHorz) {
      break;
    }
  }
  int b = height - 1;
  for (; b >= 0; b--) {
    if (ColorValue(mCurrentCharInfo->HorzColor.at(b)) >= minHorz) {
      break;
    }
  }
  int w = r - l + 1;
  int h = b - t + 1;
  mCurrentCharInfo->VertColor = mCurrentCharInfo->VertColor.mid(l, w);
  mCurrentCharInfo->HorzColor = mCurrentCharInfo->HorzColor.mid(t, h);
  mCurrentCharInfo->Value.SetSource(mCurrentCharInfo->Source, l, t, w, h);
}

void Uin::BeginTest()
{
  mTestInfoList.clear();
}

void Uin::BeginChar(const Region<uchar>& region, int whiteMin, int blackMax)
{
  mTestInfoList.append(TestInfo());
  mCurrentTestInfo = &mTestInfoList.back();

  mCurrentTestInfo->Value.SetSource(region);
  mCurrentTestInfo->WhiteMin = whiteMin;
  mCurrentTestInfo->BlackMax = blackMax;

  Recalc();
}

void Uin::TrimChar(QRect* rect)
{
  if (rect) {
    QPoint leftTop     = rect->topLeft();
    QPoint rightBottom = rect->bottomRight();
    TrimCharVert(&leftTop.rx(), &rightBottom.rx());
    TrimCharHorz(&leftTop.ry(), &rightBottom.ry());
    rect->setTopLeft(leftTop);
    rect->setBottomRight(rightBottom);
  } else {
    TrimCharVert();
    TrimCharHorz();
  }
}

void Uin::TrimCharVert(int* left, int* right)
{
  if (mCurrentTestInfo->Value.Width() < kMinRegionSize || mCurrentTestInfo->Value.Height() < kMinRegionSize) {
    return;
  }

  int begin = 0;
  int end   = mCurrentTestInfo->VertColor.size();
  while (begin < end - 3) {
    if (mCurrentTestInfo->VertColor.at(begin).Black + mCurrentTestInfo->VertColor.at(begin).Gray == 0) {
      begin++;
    } else if (mCurrentTestInfo->VertColor.at(begin + 1).Black + mCurrentTestInfo->VertColor.at(begin + 1).Gray == 0) {
      begin += 2;
    } else if (mCurrentTestInfo->VertColor.at(begin).Black == 0 && mCurrentTestInfo->VertColor.at(begin + 1).Black == 0) {
      begin++;
    } else if (mCurrentTestInfo->VertColor.at(begin + 1).Black == 0 && mCurrentTestInfo->VertColor.at(begin + 2).Black == 0) {
      begin += 2;
    } else {
      break;
    }
  }
  while (end >= 3) {
    if (mCurrentTestInfo->VertColor.at(end - 1).Black + mCurrentTestInfo->VertColor.at(end - 1).Gray == 0) {
      end--;
    } else if (mCurrentTestInfo->VertColor.at(end - 2).Black + mCurrentTestInfo->VertColor.at(end - 2).Gray == 0) {
      end -= 2;
    } else if (mCurrentTestInfo->VertColor.at(end - 1).Black == 0 && mCurrentTestInfo->VertColor.at(end - 2).Black == 0) {
      end--;
    } else if (mCurrentTestInfo->VertColor.at(end - 2).Black == 0 && mCurrentTestInfo->VertColor.at(end - 3).Black == 0) {
      end -= 2;
    } else {
      break;
    }
  }
  if (begin > 0 || end < mCurrentTestInfo->VertColor.size()) {
    if (end - begin >= kMinRegionSize) {
      mCurrentTestInfo->VertColor = mCurrentTestInfo->VertColor.mid(begin, end - begin);
      mCurrentTestInfo->Value.SetSource(mCurrentTestInfo->Value, begin, 0, end - begin, mCurrentTestInfo->Value.Height());
      if (left) {
        *left += begin;
      }
      if (right) {
        *right -= mCurrentTestInfo->VertColor.size() - end;
      }
    } else {
      mCurrentTestInfo->Value.SetSize(1, 1);
      *mCurrentTestInfo->Value.Data(0, 0) = 255;
      if (left) {
        *left = 0;
      }
      if (right) {
        *right = 0;
      }
    }
    mNeedRecalc = true;
  }
}

void Uin::TrimCharHorz(int* top, int* bottom)
{
  if (mCurrentTestInfo->Value.Width() < kMinRegionSize || mCurrentTestInfo->Value.Height() < kMinRegionSize) {
    return;
  }

  int begin = 0;
  int end   = mCurrentTestInfo->HorzColor.size();
  while (begin < end - 3) {
    if (mCurrentTestInfo->HorzColor.at(begin).Black + mCurrentTestInfo->HorzColor.at(begin).Gray == 0) {
      begin++;
    } else if (mCurrentTestInfo->HorzColor.at(begin + 1).Black + mCurrentTestInfo->HorzColor.at(begin + 1).Gray == 0) {
      begin += 2;
    } else if (mCurrentTestInfo->HorzColor.at(begin).Black == 0 && mCurrentTestInfo->HorzColor.at(begin + 1).Black == 0) {
      begin++;
    } else if (mCurrentTestInfo->HorzColor.at(begin + 1).Black == 0 && mCurrentTestInfo->HorzColor.at(begin + 2).Black == 0) {
      begin += 2;
    } else {
      break;
    }
  }
  while (end >= 3) {
    if (mCurrentTestInfo->HorzColor.at(end - 1).Black + mCurrentTestInfo->HorzColor.at(end - 1).Gray == 0) {
      end--;
    } else if (mCurrentTestInfo->HorzColor.at(end - 2).Black + mCurrentTestInfo->HorzColor.at(end - 2).Gray == 0) {
      end -= 2;
    } else if (mCurrentTestInfo->HorzColor.at(end - 1).Black == 0 && mCurrentTestInfo->HorzColor.at(end - 2).Black == 0) {
      end--;
    } else if (mCurrentTestInfo->HorzColor.at(end - 2).Black == 0 && mCurrentTestInfo->HorzColor.at(end - 3).Black == 0) {
      end -= 2;
    } else {
      break;
    }
  }
  if (begin > 0 || end < mCurrentTestInfo->HorzColor.size()) {
    if (top) {
      *top += begin;
    }
    if (bottom) {
      *bottom -= mCurrentTestInfo->HorzColor.size() - end;
    }
    if (end - begin >= kMinRegionSize) {
      mCurrentTestInfo->HorzColor = mCurrentTestInfo->HorzColor.mid(begin, end - begin);
      mCurrentTestInfo->Value.SetSource(mCurrentTestInfo->Value, 0, begin, mCurrentTestInfo->Value.Width(), end - begin);
    } else {
      mCurrentTestInfo->Value.SetSize(1, 1);
      *mCurrentTestInfo->Value.Data(0, 0) = 255;
    }
    mNeedRecalc = true;
  }
}

void Uin::TestChar()
{
  if (mCurrentTestInfo->Value.Width() < kMinRegionSize || mCurrentTestInfo->Value.Height() < kMinRegionSize) {
    return;
  }
  if (mNeedRecalc) {
    Recalc();
  }

  mCurrentTestInfo->BestMatch = 0;

  foreach (const CharInfo& charInfo, mCharInfoList) {
    qreal bestMatch = MatchValue(charInfo, 0, 0);
//    qreal bestMatch = MatchHyst(charInfo, 0, 0);
//    if (bestMatch >= 1) {
//      bestMatch = MatchValue(charInfo, 0, 0);
//    }
    mCurrentTestInfo->CharMatch.insert(charInfo.Char, bestMatch);
    if (bestMatch > mCurrentTestInfo->BestMatch) {
      mCurrentTestInfo->BestMatch = bestMatch;
      mCurrentTestInfo->BestChar = charInfo.Char;
    }
  }
}

void Uin::TestCharRadius(int radius)
{
  foreach (const CharInfo& charInfo, mCharInfoList) {
    qreal bestMatch = 0;
    int bestI = 0;
    int bestJ = 0;
    for (int j = -radius; j <= radius; j++) {
      for (int i = -radius; i <= radius; i++) {
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
          qreal match = MatchValue(charInfo, bestI + i, bestJ + j);
          if (match > bestMatch) {
            bestMatch = match;
          }
        }
      }
    }
    mCurrentTestInfo->CharMatch.insert(charInfo.Char, bestMatch);
  }
}

qreal Uin::GetBestMatch()
{
  return mCurrentTestInfo->BestMatch;
}

QChar Uin::GetBestChar()
{
  return mCurrentTestInfo->BestChar;
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
  debug->SetData(255);

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
  debug->SetData(255);
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

bool Uin::DumpDigits(Region<uchar>* debug)
{
  int width = 0;
  int height = 0;
  foreach (const TestInfo& testInfo, mTestInfoList) {
    width  = qMax(width, testInfo.Value.Width() + 8);
    height = qMax(height, testInfo.Value.Height());
  }

  int columns = qMax(12, mTestInfoList.size() / 4);
  int rows = (mTestInfoList.size() + columns - 1) / columns;
  debug->SetSize(columns * width, rows * 3*height);
  debug->SetData(128);
  for (int i = 0; i < mTestInfoList.size(); i++) {
    const TestInfo& info = mTestInfoList.at(i);
    qreal bestMatch = 0;
    QChar bestChar  = QChar('z');
    for (auto itr = info.CharMatch.begin(); itr != info.CharMatch.end(); itr++) {
      if (itr.value() > bestMatch) {
        bestMatch = itr.value();
        bestChar  = itr.key();
      }
    }

//    if (bestMatch < kMinDigitMatch) {
//      continue;
//    }
    int x = (i % columns) * width;
    int y = (i / columns) * 3*height;
    debug->Copy(QPoint(x + (width - info.Value.Width())/2, y), info.Value, info.Value.Rect());
    foreach (const CharInfo& chInfo, mCharInfoList) {
      if (chInfo.Char == bestChar) {
        for (int j = 0; j < height; j++) {
          int jj = chInfo.Value.Height() * j / height;
          uchar* dbg = debug->Data(x, y + height + j);
          for (int i = 0; i < width; i++) {
            int ii = chInfo.Value.Width() * i / width;
            *dbg = *chInfo.Value.Data(ii, jj);
            dbg++;
          }
        }
        break;
      }
    }

    int matchValue = qBound(0, (int)((info.BestMatch - 1.0) * 333.0), 255);
    debug->FillRect(QRect(x, y + height*2, width, height), matchValue);
  }
  return true;
}

void Uin::Recalc()
{
  const Region<uchar>& region = mCurrentTestInfo->Value;

  mCurrentTestInfo->HorzColor.resize(region.Height());
  mCurrentTestInfo->VertColor.resize(region.Width());

  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    Color* horzValue = &mCurrentTestInfo->HorzColor[j];
    Color* vertValue = &mCurrentTestInfo->VertColor[0];

    for (int i = 0; i < region.Width(); i++) {
      if (*src > mCurrentTestInfo->WhiteMin) {
        horzValue->White++;
        vertValue->White++;
      } else if (*src <= mCurrentTestInfo->BlackMax) {
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

  mNeedRecalc = false;
}

qreal Uin::MatchHyst(const CharInfo& charInfo, int xDisp, int yDisp)
{
  qreal kx = (qreal)charInfo.Value.Width() / mCurrentTestInfo->Value.Width();
  qreal ky = (qreal)charInfo.Value.Height() / mCurrentTestInfo->Value.Height();
  qreal match = 0;

  const Color* horzColor = mCurrentTestInfo->HorzColor.constData();
  for (int j = 0; j < mCurrentTestInfo->Value.Height(); j++) {
    int y = (0.5 + j + yDisp) * ky - 0.5;
    if (y < 0 || y >= charInfo.HorzColor.size()) {
      continue;
    }
    const Color& charColor = charInfo.HorzColor.at(y);
    qreal missB = qAbs(kx * horzColor->Black - charColor.Black);
    qreal missW = qAbs(kx * horzColor->White - charColor.White);
//    qreal missG = qAbs(k * horzColor->Gray - charColor.Gray);
    match += (1.0 - (missB + missW) / charInfo.Value.Width()) / mCurrentTestInfo->Value.Height();

    horzColor++;
  }

  const Color* vertColor = mCurrentTestInfo->VertColor.constData();
  for (int i = 0; i < mCurrentTestInfo->Value.Width(); i++) {
    int x = (0.5 + i + xDisp) * kx - 0.5;
    if (x < 0 || x >= charInfo.VertColor.size()) {
      continue;
    }
    const Color& charColor = charInfo.VertColor.at(x);
    qreal missB = qAbs(ky * vertColor->Black - charColor.Black);
    qreal missW = qAbs(ky * vertColor->White - charColor.White);
//    qreal missG = qAbs(k * vertColor->Gray - charColor.Gray);
    match += (1.0 - (missB + missW) / charInfo.Value.Height()) / mCurrentTestInfo->Value.Width();

    vertColor++;
  }

  return match;
}

qreal Uin::MatchValue(const Uin::CharInfo& charInfo, int xDisp, int yDisp)
{
  qreal kx = (qreal)charInfo.Value.Width() / mCurrentTestInfo->Value.Width();
  qreal ky = (qreal)charInfo.Value.Height() / mCurrentTestInfo->Value.Height();
  int match = 0;
  int count = 0;
  for (int j = 0; j < mCurrentTestInfo->Value.Height(); j++) {
    int y = (0.5 + j + yDisp) * ky - 0.5;
    if (y < 0 || y >= charInfo.HorzColor.size()) {
      continue;
    }
    for (int i = 0; i < mCurrentTestInfo->Value.Width(); i++) {
      int x = (0.5 + i + xDisp) * kx - 0.5;
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

int Uin::ColorValue(const Uin::Color& a)
{
  return a.Gray + 4*a.Black;
}

bool Uin::ColorComp(const Uin::Color& a, const Uin::Color& b)
{
  return ColorValue(a) < ColorValue(b);
}


Uin::Uin(Analyser* _Analyser)
  : mAnalyser(_Analyser)
  , mSourceData(nullptr)
{
  Q_INIT_RESOURCE(Analyser);
}

Uin::~Uin()
{
}

