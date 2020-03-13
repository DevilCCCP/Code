#include <QColor>

#include "Uin.h"


const int kCharCut = 3;

void Uin::AddChar(const QChar& _Char, const QImage& _Image)
{
  mCharsMap.append(CharMap());
  CharMap* chmap = &mCharsMap.last();
  chmap->Char = _Char;
  int width = _Image.width();
  int height = _Image.height();
  chmap->Source.SetSize(width, height);
  Region<uchar>* region = &chmap->Source;
  mCharLine.resize(0);
  mCharLine.resize(region->Height(), (int)0);
  mCharColumn.resize(0);
  mCharColumn.resize(region->Width(), (int)0);
  int* line = mCharLine.data();
  QList<QPoint> currentPoints;
  for (int j = 0; j < height; j++) {
    const QRgb* data = reinterpret_cast<const QRgb*>(_Image.scanLine(j));
    uchar* dst = region->Line(j);
    int* column = mCharColumn.data();
    for (int i = 0; i < width; i++) {
      if (QColor::fromRgb(*data).value() < 0x80) {
        *dst = 0;
        *line = kCharCut;
        *column = kCharCut;
        currentPoints.append(QPoint(i, j));
      } else {
        *dst = 0xff;
      }

      data++;
      dst++;
      column++;
    }
    line++;
  }

  int dx, dy;
  if (CutChar(chmap->Source, chmap->Map, &dx, &dy)) {
    foreach (const QPoint& p, currentPoints) {
      const_cast<QPoint&>(p).rx() -= dx;
      const_cast<QPoint&>(p).ry() -= dy;
    }
  } else {
    chmap->Map.SetSource(chmap->Source, 0, 0, chmap->Source.Width(), chmap->Source.Height());
  }
  region = &chmap->Map;
  width = chmap->Map.Width();
  height = chmap->Map.Height();

  int stride = region->Stride();
  while (!currentPoints.isEmpty()) {
    QList<QPoint> nextPoints;
    foreach (const QPoint& p, currentPoints) {
      int i = p.x();
      int j = p.y();
      uchar* pdata = region->Data(i, j);
      uchar weight = *pdata;
      if (weight == 0xff) {
        continue;
      }
      if (i > 0 && weight + 1 < pdata[-1]) {
        pdata[-1] = weight + 1;
        nextPoints.append(QPoint(i - 1, j));
      }
      if (j > 0 && weight + 1 < pdata[-stride]) {
        pdata[-stride] = weight + 1;
        nextPoints.append(QPoint(i, j - 1));
      }
      if (i < width - 1 && weight + 1 < pdata[1]) {
        pdata[1] = weight + 1;
        nextPoints.append(QPoint(i + 1, j));
      }
      if (j < height - 1 && weight + 1 < pdata[stride]) {
        pdata[stride] = weight + 1;
        nextPoints.append(QPoint(i, j + 1));
      }
    }
    currentPoints = nextPoints;
  }
}

bool Uin::Calc()
{
  if (mRegion->Width() <= 2 || mRegion->Height() < 6 || mCharsMap.isEmpty()) {
    return false;
  }

  mCharHyst.Clear();
  CalcRegionHyst(*mRegion, mCharHyst);

  MkChar3Color();

  CalcQuality();

  mDebugRegion = &mCharRegion;

  return mQuality > 0;
}

void Uin::MkChar3Color()
{
  int low = mCharHyst.GetValue(50);
  int high = mCharHyst.GetValue(750);
  int d = high - low;
  low += d/2;
  high -= d/5;

  mCharRegion.SetSize(mRegion->Width(), mRegion->Height());
  mCharLine.resize(0);
  mCharLine.resize(mRegion->Height(), (int)0);
  mCharColumn.resize(0);
  mCharColumn.resize(mRegion->Width(), (int)0);
  int* line = mCharLine.data();
  for (int j = 0; j < mRegion->Height(); j++) {
    const uchar* src = mRegion->Line(j);
    uchar* dst = mCharRegion.Line(j);
    int* column = mCharColumn.data();
    for (int i = 0; i < mRegion->Width(); i++) {
      if (*src < low) {
        *dst = 0;
        (*column) += kCharCut;
        (*line) += kCharCut;
      } else if (*src < high) {
        *dst = 0x80;
        (*column)++;
        (*line)++;
      } else {
        *dst = 0xff;
      }

      src++;
      dst++;
      column++;
    }
    line++;
  }

  CutChar(mCharRegion, mCharMapRegion);
}

void Uin::CalcQuality()
{
  mQuality = 0;
  mChar = '0';

  static int index = 0;
  int y = 1;
  QString dbg;
  foreach (const CharMap& chmap, mCharsMap) {
    int quality = CalcQualityOne(chmap);
    dbg.append(QString("'%1': %2\n").arg(chmap.Char).arg(quality));
    if (quality > mQuality) {
      mQuality = quality;
      mChar = chmap.Char;
    }
  }
  dbg.append("done");
  index++;
}

qreal inline QualityWhite(const uchar& len)
{
  return len > 0? 1.0: 0.3;
}

qreal inline QualityGray(const uchar& len)
{
  if (len == 0) {
    return 0.7;
  } else if (len <= 2) {
    return 1.0;
  } else {
    return qMax(0.2, 1.0 - 0.2 * (len - 2));
  }
}

qreal inline QualityBlack(const uchar& len)
{
  switch (len) {
  case 0: return 4.0;
  case 1: return 2.0;
  case 2: return 0.5;
  case 3: return 0.2;
  default: return 0;
  }
}

int Uin::CalcQualityOne(const Uin::CharMap& chmap)
{
  const Region<uchar>* digitMap = &chmap.Map;

  qreal quality = 0;
  qreal kWh1 = (qreal)digitMap->Width() / digitMap->Height();
  qreal kWh2 = (qreal)mCharMapRegion.Width() / mCharMapRegion.Height();
  if (kWh1 < 0.7*kWh2 || kWh1 > 1.4*kWh2) {
    return 0;
  }

  qreal kw = (qreal)digitMap->Width() / mCharMapRegion.Width();
  qreal kh = (qreal)digitMap->Height() / mCharMapRegion.Height();
  int iFrom = (kw > 1)? 0: 1;
  int iTo = (kw > 1)? mCharMapRegion.Width(): mCharMapRegion.Width() - 1;
  int jFrom = (kh > 1)? 0: 1;
  int jTo = (kh > 1)? mCharMapRegion.Height(): mCharMapRegion.Height() - 1;
  for (int j = jFrom; j < jTo; j++) {
    const uchar* src = mCharMapRegion.Data(iFrom, j);
    qreal y = (j + 0.5) * kh - 0.5;
    int j1 = (int)y;
    int j2 = j1 + 1;
    qreal ky1 = j2 - y;
    qreal ky2 = y - j1;
    for (int i = iFrom; i < iTo; i++) {
      qreal x = (i + 0.5) * kw - 0.5;
      int i1 = (int)x;
      int i2 = i1 + 1;
      qreal kx1 = i2 - x;
      qreal kx2 = x - i1;

      if (*src == 0xff) {
        quality += ky1 * kx1 * QualityWhite(*digitMap->Data(i1, j1));
        quality += ky1 * kx2 * QualityWhite(*digitMap->Data(i2, j1));
        quality += ky2 * kx1 * QualityWhite(*digitMap->Data(i1, j2));
        quality += ky2 * kx2 * QualityWhite(*digitMap->Data(i2, j2));
      } else if (*src >= 0x80) {
        quality += ky1 * kx1 * QualityGray(*digitMap->Data(i1, j1));
        quality += ky1 * kx2 * QualityGray(*digitMap->Data(i2, j1));
        quality += ky2 * kx1 * QualityGray(*digitMap->Data(i1, j2));
        quality += ky2 * kx2 * QualityGray(*digitMap->Data(i2, j2));
      } else {
        quality += ky1 * kx1 * QualityBlack(*digitMap->Data(i1, j1));
        quality += ky1 * kx2 * QualityBlack(*digitMap->Data(i2, j1));
        quality += ky2 * kx1 * QualityBlack(*digitMap->Data(i1, j2));
        quality += ky2 * kx2 * QualityBlack(*digitMap->Data(i2, j2));
      }

      src++;
    }
  }
  return quality * 100.0 / ((iTo - iFrom) * (jTo - jFrom));
}

void Uin::CalcRegionHyst(const Region<uchar>& region, Hyst& hyst)
{
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    for (int i = 0; i < region.Width(); i++) {
      hyst.Inc(*src);
      src++;
    }
  }
}

bool Uin::CutChar(const Region<uchar>& region, Region<uchar>& regionResized, int* dx, int* dy)
{
  int x = 0;
  while (x < region.Width() && mCharColumn[x] < kCharCut) {
    x++;
  }
  if (x >= region.Width()) {
    return false;
  }

  int y = 0;
  while (y < region.Height() && mCharLine[y] < kCharCut) {
    y++;
  }
  if (y >= region.Height()) {
    return false;
  }

  int width = region.Width();
  while (mCharColumn[width - 1] < kCharCut) {
    width--;
  }
  width -= x;

  int height = region.Height();
  while (mCharLine[height - 1] < kCharCut) {
    height--;
  }
  height -= y;

  if (x > 0 || width < region.Width() || y > 0 || height < region.Height()) {
    regionResized.SetSource(region, x, y, width, height);
    if (dx) {
      *dx = x;
    }
    if (dy) {
      *dy = y;
    }
    return true;
  }
  return false;
}


Uin::Uin()
  : mRegion(nullptr)
{
}

Uin::~Uin()
{
}

