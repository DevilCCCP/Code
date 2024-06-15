#include "UinPre.h"
#include "Uin.h"


const int kMinUinHeight = 10;
const int kMinUinDetectHeight = 3;
const int kMinHeight  = 3;
const int kMinValue   = 2;
const int kMinHystCount = 200;
const int kTopThresholdDefault = 5;
const int kNearLength = 32;
// 3:5 - .123..
// 5:6 - a...co

int UinPre::MinHeight()
{
  return kMinUinHeight;
}

void UinPre::Update()
{
  mTopThreshold = (mTopHyst.TotalCount() > kMinHystCount)? mTopHyst.GetValue(950) / 2: kTopThresholdDefault;
}

void UinPre::Clear()
{
  mTopHyst.Clear();

  Update();
}

void UinPre::Clear1()
{
  mObjsList.clear();
  mCurObjsList.clear();
  mDebugText.clear();
}

void UinPre::Clear2()
{
  mObjHyst.Clear();
  mDigits.clear();
  mDebugText.clear();
}

void UinPre::Calc(const Region<uchar>* region)
{
  CalcStage1(region);

  for (int i = 0; i < mObjsList.size(); i++) {
    const Obj& obj = mObjsList.at(i);
    if (IsValidObj(obj)) {
      CalcStage2(obj.Dimentions);
    }
  }
}

void UinPre::CalcVector(const QVector<uchar>& line, QVector<uchar>& marks)
{
  marks.fill(0, line.size());
  if (line.size() <= 1) {
    return;
  }

  CalcLine(line.constData(), marks.data(), line.size());
}

void UinPre::CalcLine(const uchar* src, uchar* marks, int size)
{
  mSrc = src;
  mResultData = marks;
  mSrcSize = size;

  *mResultData = 0;
  mResultData++;
  mSrcl = mSrc;
  mSrc++;
  mSrcSize--;

  // Left         Top       Right
  // white... black mark ...white
  // 255  down     0     up   255
  mDirection = eLeft;
  mHeight = 0;
  mLength = 0;
  mLeftTop  = 0;
  mRightTop = 0;
  mLastTop = -2 * kNearLength;
  mLastValue = 0;
  for (int i = 0; i < mSrcSize; i++) {
    if (*mSrcl >= *mSrc) { // down (Left -> Top)
      int h = *mSrcl - *mSrc;
      switch (mDirection) {
      case eLeft:
        if (2*mHeight < h) {
          Reset(h);
        } else if (2*h < mHeight) {
          mDirection = eTop;
          mHeightL = mHeight;
          mLeftTop = i;
        } else {
          mHeight = qMax(mHeight, h);
        }
        break;

      case eTop:
        if (4*h > mHeight) {
          Reset(h);
        }
        break;

      case eRight:
        ApplyTop();
        Reset(h);
        break;
      }
    } else { // up (Top -> Right)
      int h = *mSrc - *mSrcl;
      switch (mDirection) {
      case eRight:
        if (2*mHeight < h) {
          Reset();
        } else if (2*h < mHeight) {
          ApplyTop();
          Reset();
        } else {
          mHeight = qMax(mHeight, h);
          mHeightR = qMax(mHeightR, h);
        }
        break;

      case eTop:
        if (4*h < mHeight) {
        } else if (2*mHeight < h) {
          Reset();
        } else {
          mDirection = eRight;
          mHeightL = mHeight;
          mHeightR = h;
          mRightTop = i;
        }
        break;

      case eLeft:
        if (4*h < mHeight) {
          mDirection = eTop;
          mHeightL = mHeight;
          mLeftTop = i;
        } else if (2*mHeight < h) {
          Reset();
        } else {
          mDirection = eRight;
          mLeftTop = i;
          mRightTop = i;
          mHeightL = mHeight;
          mHeightR = h;
          mHeight = qMax(mHeight, h);
        }
        break;
      }
    }

    mLength++;
    mSrcl++;
    mSrc++;
  }
}

void UinPre::CalcStage1(const Region<uchar>* region)
{
  Clear1();

  mSourceData = region;

  Update();
  mPreData.SetSize(mSourceData->Width(), mSourceData->Height());
  mPreData.ZeroData();
  for (int j = 0; j < mSourceData->Height(); j++) {
    const uchar* src = mSourceData->Line(j);
    uchar* dst = mPreData.Line(j);
    CalcLine(src, dst, mSourceData->Width());
    if (j > 1) {
      uchar* dstp = mPreData.Line(j - 1);
      MergeLines(dst, dstp, mSourceData->Width());
    }
  }

  for (int j = mSourceData->Height() - 1; j > 0; j--) {
    uchar* dst  = mPreData.Line(j);
    uchar* dstp = mPreData.Line(j - 1);
    MergeLinesBack(dst, dstp, mSourceData->Width());
  }

  SelectLineBegin();
  for (int j = 0; j < mSourceData->Height(); j++) {
    uchar* dst  = mPreData.Line(j);
    SelectLine(j, dst, mSourceData->Width());
  }
  SelectLineEnd();
}

void UinPre::CalcStage2(const Rectangle& rect)
{
  Clear2();
  if (!mUin) {
    mUin.reset(new Uin(0));
    const char kSymbols[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
                              , 'A', 'B', 'C', 'D', 'E', 'H', 'K', 'M', 'O', 'P', 'T', 'X', 'Y', '\0' };
    for (int i = 0; kSymbols[i]; i++) {
      QImage img(QString(":/UinFont/Uin %1.png").arg(kSymbols[i]));
      if (!img.isNull()) {
        mUin->AddChar(QChar(kSymbols[i]), img);
      }
    }
  }

  int deltaX = (rect.Width() + 2) / 4;
  int deltaY = (rect.Height() + 2) / 4;
  mObjSource.SetSource(*mSourceData, rect.Left + deltaX, rect.Top + deltaY, rect.Width() - 2*deltaX, rect.Height() - 2*deltaY);
  CalcRegionHyst(mObjSource, mObjHyst);

  Rectangle rectMore;
  rectMore.Left = qMax(0, rect.Left - deltaX);
  rectMore.Right = qMin(mSourceData->Width() - 1, rect.Right + deltaX);
  rectMore.Top = qMax(0, rect.Top - deltaY);
  rectMore.Bottom = qMin(mSourceData->Height() - 1, rect.Bottom + deltaY);
  mObjSource.SetSource(*mSourceData, rectMore.Left, rectMore.Top, rectMore.Width(), rectMore.Height());
  mObjValue.SetSize(mObjSource.Width() + 1, mObjSource.Height() + 2);

//  CalcRegionHyst(mObjSource, mObjHyst);
  CalcBackFront();
  CalcDigitSpaces();

#ifndef QT_NO_DEBUG
  // debug
  mObjValue.SetSize(400, 200);
  mObjValue.FillData(0x40);
  mDebugDigitPos = 0;
#endif

  CalcDigits();
  //  CalcDigitMass();
}

void UinPre::Calc3Color(const Region<uchar>& regionSrc, Region<uchar>& regionDst)
{
  Hyst hyst;
  CalcRegionHyst(regionSrc, hyst);

  regionDst.SetSize(regionSrc.Width(), regionSrc.Height());
  Mk3Color(hyst, regionSrc, regionDst);
}

void UinPre::MergeLines(uchar* line, uchar* linep, int size)
{
  linep++;
  for (int i = 0; i < size - 2; i++) {
    if (*linep) {
      int baseValue = *linep;
      int sumValue = baseValue;
      if (line[0] || line[1] || line[2]) {
        sumValue++;
        uchar val = (uchar)qMin(kMinUinHeight, sumValue);
        *linep = val;
        if (line[0]) {
          line[0] = val;
        } if (line[1]) {
          line[1] = val;
        } if (line[2]) {
          line[2] = val;
        }
      }
    }

    linep++;
    line++;
  }
}

void UinPre::MergeLinesBack(uchar* line, uchar* linep, int size)
{
  linep++;
  for (int i = 0; i < size - 2; i++) {
    if (*linep) {
      uchar val = qMax(qMax(line[0], line[1]), qMax(*linep, line[2]));
      if (val < kMinUinDetectHeight) {
        val = 1;
      }
      *linep = val;
    }

    linep++;
    line++;
  }
}

void UinPre::SelectLineBegin()
{
  mObjsList.clear();
  mCurObjsList.clear();
}

void UinPre::SelectLine(int j, uchar* line, int size)
{
  int last = 0;
  int limit = -1;
  for (int i = 0; i < size; i++) {
    if (*line >= kMinHeight) {
      if (i <= limit) {
        int fill = i - last - 1;
        if (fill > 0) {
          Obj* hitObj = nullptr;
          for (auto itr = mCurObjsList.begin(); itr != mCurObjsList.end(); ) {
            Obj* obj = &*itr;
            if (obj->Dimentions.Left <= i && last <= obj->Dimentions.Right) {
              obj->Dimentions.Left = qMin(obj->Dimentions.Left, last);
              obj->Dimentions.Right = qMax(obj->Dimentions.Right, i);
              obj->Dimentions.Bottom = j;
              if (hitObj) {
                hitObj->Dimentions.Left = qMin(obj->Dimentions.Left, hitObj->Dimentions.Left);
                hitObj->Dimentions.Right = qMax(obj->Dimentions.Right, hitObj->Dimentions.Right);
                hitObj->Dimentions.Top = qMin(obj->Dimentions.Top, hitObj->Dimentions.Top);
                itr = mCurObjsList.erase(itr);
                continue;
              } else {
                hitObj = obj;
              }
            }
            itr++;
          }

          if (!hitObj) {
            mCurObjsList.append(Obj());
            Obj* obj = &mCurObjsList.last();
            obj->Dimentions.Top = obj->Dimentions.Bottom = j;
            obj->Dimentions.Left = last;
            obj->Dimentions.Right = i;
          }

          line -= fill;
          for (int k = 0; k < fill; k++) {
            *line = 1;
            line++;
          }
        }
      }

      last = i;
      limit = i + kNearLength;
    }

    line++;
  }

  for (auto itr = mCurObjsList.begin(); itr != mCurObjsList.end(); ) {
    Obj* obj = &*itr;
    if (obj->Dimentions.Bottom < j) {
      mObjsList.append(*obj);
      itr = mCurObjsList.erase(itr);
    } else {
      itr++;
    }
  }
}

void UinPre::SelectLineEnd()
{
  foreach (const Obj& obj, mCurObjsList) {
    mObjsList.append(obj);
  }

  mCurObjsList.clear();
  mNextRect = 0;
}

bool UinPre::NextRect(Rectangle& rect)
{
  while (mNextRect < mObjsList.size()) {
    const Obj& obj = mObjsList.at(mNextRect++);
    if (IsValidObj(obj)) {
      rect = obj.Dimentions;
      return true;
    }
  }
  return false;
}

void UinPre::DebugValue(Region<uchar>* regionValue)
{
  regionValue->SetSource(mPreData, 0, 0, mPreData.Width(), mPreData.Height());
}

void UinPre::DebugValue2(Region<uchar>* regionValue)
{
  regionValue->SetSource(mObjValue, 0, 0, mObjValue.Width(), mObjValue.Height());
}

void UinPre::ApplyTop()
{
//  int value = qMin(mHeightL, mHeightR) / mLength;
  int value = qMin(mHeightL, mHeightR) / (mLength * mLength);
  if (value) {
    mTopHyst.Inc(value);
  }
//  if (value < mTopThreshold) {
//    return;
//  } else if (value < 2 * mTopThreshold) {
//    value = 1;
//  } else {
//    value = 2;
//  }

  mResultData[mLeftTop - 1] = mResultData[mRightTop - 1] = value;
}

void UinPre::Reset(int h)
{
  mDirection = eLeft;
  mLength = 0;
  mHeight = h;
}

void UinPre::Reset()
{
  mDirection = eLeft;
  mLength = -1;
  mHeight = 0;
}

void inline SetValue(uchar value, uchar* dst, int length)
{
  dst -= length;
  for (int i = 0; i < length; i++) {
    *dst++ = value;
  }
}

void inline CopyValue(const uchar* src, uchar* dst, int length)
{
  src -= length;
  dst -= length;
  for (int i = 0; i < length; i++) {
    *dst++ = *src++;
  }
}

void UinPre::CalcBackFront()
{
  int low = mObjHyst.GetValue(50);
  int high = mObjHyst.GetValue(750);
  int d = high - low;
  low += d/3;
  int mid = (low + high)/2;

  int minLen = mObjSource.Height();
  for (int j = 0; j < mObjSource.Height(); j++) {
    const uchar* src  = mObjSource.Line(j);
    uchar* dst = mObjValue.Line(j);

    bool start = true;
    int length = 0;
    bool back = *src >= mid;
    for (int i = 0; i < mObjSource.Width(); i++) {
      if (*src < mid) {
        if (back) {
          if (start || length >= minLen) {
            SetValue(255, dst, length);
          } else {
            CopyValue(src, dst, length);
          }
          back = false;
          start = false;
          length = 0;
        }
      } else {
        if (!back) {
          if (start || length >= minLen) {
            SetValue(0, dst, length);
          } else {
            CopyValue(src, dst, length);
          }
          back = true;
          start = false;
          length = 0;
        }
      }

      src++;
      dst++;
      length++;
    }

    SetValue(back? 255: 0, dst, length);
  }

  int minHeight = (mObjSource.Height() + 3) / 6;
  mObjFront.resize(0);
  mObjFront.resize(mObjSource.Width(), Front(-1, -1));
  for (int i = 0; i < mObjSource.Width(); i++) {
    const uchar* dst  = mObjValue.Data(i, 0);
    Front* front = &mObjFront[i];
    int place = 0;
    int digit = 0;
    bool preWhite = false;
    for (int j = 0; j < mObjSource.Height(); j++) {
      switch (place) {
      case 0: // no digit
        if (*dst == 0) {
          preWhite = false;
        } else if (*dst == 255) {
          preWhite = true;
        } else {
          digit = j;
          place = 1;
        }
        break;

      case 1: // in digit
        if (*dst == 255 || *dst == 0) {
          bool postWhite = *dst == 255;
          place = 0;
          int len = j - digit;
          if (len >= minHeight) {
            int olen = front->To - front->From;
            bool ok = (len > olen);
            if (len == olen) {
              int omid = (front->To + front->From - 1);
              int mid = (digit + j - 1);
              if (qAbs(mid - mObjSource.Height()) < qAbs(omid - mObjSource.Height())) {
                ok = true;
              }
            }
            if (ok) {
              front->From = digit;
              front->To = j;
              front->Flag = (preWhite? Front::ePreWhite: 0) | (postWhite? Front::ePostWhite: 0);
              if (postWhite) {
                place = 2;
              }
            }
          }
        }
        break;

      case 2: // ends digit (post)
        if (*dst == 255) {
          front->Flag |= Front::ePostWhite;
          place = 3;
        } else if (*dst == 0) {
          place = 0;
        }
        break;

      case 3: // ends digit (total)
        if (*dst == 0) {
          place = 0;
        } else if (*dst != 255) {
          front->To = j + 1;
          front->Flag &= (~Front::ePostWhite);
          place = 2;
        }
        break;
      }

      dst += mObjValue.Stride();
    }
  }

  mDigitsStart = -1;
  mDigitsFinish = -1;
  int len = 0;
  int okStart = 0;
  for (int i = 0; i < mObjSource.Width(); i++) {
    Front& front = mObjFront[i];
    if (front.Flag & Front::ePreWhite) {
      front.From--;
    }
    if (front.Flag & Front::ePostWhite) {
      front.To++;
    }
    if (front.To - front.From >= minHeight) {
      if (!len) {
        okStart = i;
      }
      len++;
    } else if (len) {
      if (i - okStart >= minLen) {
        if (mDigitsStart < 0) {
          mDigitsStart = okStart;
        }
        mDigitsFinish = i;
        for (int ii = okStart; ii < i; ii++) {
          mObjFront[ii].Ok = true;
        }
      }
      len = 0;
    }
  }

#ifndef QT_NO_DEBUG
  // debug draw
  for (int j = 0; j < mObjSource.Height(); j++) {
    const uchar* src  = mObjSource.Line(j);
    uchar* dst = mObjValue.Line(j);
    Front* front = mObjFront.data();
    for (int i = 0; i < mObjSource.Width(); i++) {
      if (front->Ok && j >= front->From && j < front->To) {
        *dst = *src;
      } else {
        *dst = 0x00;
      }

      src++;
      dst++;
      front++;
    }
  }
#endif
}

void UinPre::CalcDigitMass()
{
//  int low = mObjHyst.GetValue(50);
//  int high = mObjHyst.GetValue(750);
//  int d = high - low;
//  low += d/3;
//  int mid = (low + high)/2;

//  mDigitMass.resize(0);
//  mDigitMass.resize(mObjSource.Width(), (int)0);
//  int k1 = qMax(2, mObjSource.Height() / 4);
//  int k2 = k1 * k1;
//  for (int j = 0; j < mObjSource.Height(); j++) {
//    const uchar* src  = mObjSource.Line(j);
//    int* mass = mDigitMass.data();
//    for (int i = 0; i < mObjSource.Width(); i++) {
//      if (*src < high) {
//        if (*src < low) {
//          *mass += k2;
//        } else if (*src < mid) {
//          *mass += k1;
//        } else {
//          *mass += 1;
//        }
//      }

//      src++;
//      mass++;
//    }
//  }

//#ifndef QT_NO_DEBUG
//  // debug draw
//  for (int j = 0; j < mObjSource.Height(); j++) {
//    const uchar* src  = mObjSource.Line(j);
//    uchar* dst = mObjValue.Line(j);
//    for (int i = 0; i < mObjSource.Width(); i++) {
//      if (*src >= high) {
//        *dst = (uchar)0xff;
//      } else if (*src >= mid) {
//        *dst = (uchar)0x80;
//      } else if (*src >= low) {
//        *dst = (uchar)0x40;
//      } else {
//        *dst = 0;
//      }

//      src++;
//      dst++;
//    }
//  }

//  {
//    int j = mObjSource.Height();
//    const int* mass = mDigitMass.data();
//    uchar* dst = mObjValue.Line(j);
//    for (int i = 0; i < mObjSource.Width(); i++) {
//      if (*mass >= k2) {
//        *dst = 0;
//      } else {
//        *dst = qMin(255, (k2 - *mass) * 255 / k2);
//      }

//      mass++;
//      dst++;
//    }
//  }
//#endif
}

void UinPre::CalcDigitSpaces()
{
  mObjHyst.Clear();
  for (int i = 0; i < mObjSource.Width(); i++) {
    Front& front = mObjFront[i];
    if (front.Ok) {
      for (int j = front.From + 1; j < front.To - 1; j++) {
        const uchar* src = mObjSource.Data(i, j);
        mObjHyst.Inc(*src);
      }
    }
  }
  int low = mObjHyst.GetValue(50);
  int high = mObjHyst.GetValue(750);
  int d = high - low;
  low += d/3;
  int mid = (low + high)/2;

  mDigitMass.resize(0);
  mDigitMass.resize(mObjSource.Width(), (int)0);
  int k1 = qMax(2, mObjSource.Height() / 4);
  int k2 = 4 * k1 * k1;
  int kMax = 2*k2;
  for (int i = 0; i < mObjSource.Width(); i++) {
    Front& front = mObjFront[i];
    if (front.Ok) {
      int* mass = &mDigitMass[i];
      for (int j = front.From + 1; j < front.To - 1; j++) {
        const uchar* src = mObjSource.Data(i, j);
        if (*src < high) {
          if (*src < low) {
            *mass += k2;
          } else if (*src < mid) {
            *mass += k1;
          } else {
            *mass += 1;
          }
        }
      }
    }
  }

  mDigitSpaceHyst.Clear();
  for (int i = 0; i < mObjSource.Width(); i++) {
    Front& front = mObjFront[i];
    if (front.Ok) {
      int normalMass = qMin(mDigitMass[i], kMax) * 255 / kMax;
      mDigitSpaceHyst.Inc(normalMass);
      mDigitMass[i] = normalMass;
    }
  }

#ifndef QT_NO_DEBUG
  // debug
  mObjValue.FillData(0xcf);
  for (int i = 0; i < mObjSource.Width(); i++) {
    Front& front = mObjFront[i];
    if (front.Ok) {
      for (int j = front.From; j < front.To; j++) {
        const uchar* src = mObjSource.Data(i, j);
        uchar* dst = mObjValue.Data(i, j);

        if (*src >= mid) {
          *dst = (*src >= high)? 0xff: 0x80;
        } else {
          *dst = (*src >= low)? 0x40: 0x00;
        }
      }
    }
  }
#endif
}

void UinPre::CalcDigits()
{
  if (mDigitsStart < 0) {
    return;
  }

  mSpaceMax = mDigitSpaceHyst.GetValue(900);
  for (int startTest = mDigitsStart; startTest < mDigitsFinish; ) {
    FindNextDigit(startTest);
  }

//#ifndef QT_NO_DEBUG
//  // debug
//  int y = 1;
//  {
//    int maxSpace = mDigitSpaceHyst.GetValue(900);
//    int j = mObjSource.Height();
//    const int* mass = mDigitMass.data();
//    uchar* dst = mObjValue.Line(j);
//    for (int i = 0; i < mObjSource.Width(); i++) {
//      if (*mass >= mSpaceMax) {
//        *dst = 0;
//      } else if (*mass <= mSpaceMax) {
//        *dst = 255;
//      } else {
//        *dst = 50 + qMin(100, (maxSpace - *mass) * 100 / maxSpace);
//      }
//
//      mass++;
//      dst++;
//    }
//  }
//#endif
}

void UinPre::FindNextDigit(int& pos)
{
  mTestDigit.Start = pos;
  mTestDigit.Finish = pos;
  pos = mDigitsFinish;
  int spaceMass = mSpaceMax;
  int state = 0;
  int maxHeight = 0;
  int maxLength = 20;
  for (int i = mTestDigit.Start; i < mDigitsFinish; i++) {
    Front& front = mObjFront[i];
    if (!front.Ok) {
      mTestDigit.Finish = i;
      if (state < 2) {
        for (; i < mDigitsFinish; i++) {
          if (front.Ok) {
            break;
          }
        }
        pos = i;
      }
      break;
    }

    if (mDigitMass[i] > mSpaceMax && front.From - front.To > maxHeight) {
      maxHeight = front.From - front.To;
      maxLength = maxHeight;
    }

    switch (state) {
    case 0: // pre mass
      if (mDigitMass[i] >= mSpaceMax) {
        state = 1;
      }
      break;

    case 1: // 1'st mass
      if (mDigitMass[i] < mSpaceMax) {
        spaceMass = mDigitMass[i];
        mTestDigit.Finish = i;
        state = 2;
      }
      break;

    case 2: // after 1'st mass
      if (mDigitMass[i] < spaceMass) {
        spaceMass = mDigitMass[i];
        mTestDigit.Finish = i;
      } else if (mDigitMass[i] >= mSpaceMax) {
        pos = mTestDigit.Finish;
        state = 3;
        CalcTestDigit();
        spaceMass = mSpaceMax;
      }
      break;

    case 3: // in some mass
      if (mDigitMass[i] < mSpaceMax) {
        spaceMass = mDigitMass[i];
        mTestDigit.Finish = i;
        state = 4;
      }
      break;

    case 4: // after some mass
      if (mDigitMass[i] < spaceMass) {
        spaceMass = mDigitMass[i];
        mTestDigit.Finish = i;
      } else if (mDigitMass[i] >= mSpaceMax) {
        state = 3;
        CalcTestDigit();
        spaceMass = mSpaceMax;
      }
      break;
    }

    if (i - mTestDigit.Start > maxLength) {
      break;
    }
  }
  if (state == 2 || state == 4) {
    CalcTestDigit();
  }
}

void UinPre::CalcTestDigit()
{
  mTestDigit.Top = mObjSource.Height();
  mTestDigit.Bottom = 0;
  for (int i = mTestDigit.Start; i < mTestDigit.Finish; i++) {
    Front& front = mObjFront[i];
    mTestDigit.Top = qMin(mTestDigit.Top, front.From);
    mTestDigit.Bottom = qMax(mTestDigit.Bottom, front.To);
  }

  Region<uchar> digit;
//  digit.SetSource(mObjSource, mTestDigit.Start, mTestDigit.Top, mTestDigit.Finish - mTestDigit.Start, mTestDigit.Bottom - mTestDigit.Top);
//  mUin->setRegion(&digit);
//
//  if (mUin->Calc()) {
//    mTestDigit.Quality = mUin->getQuality();
//    mTestDigit.Char    = mUin->getChar();
//    mDigits.append(mTestDigit);
//    mDebugText.append(QString("'%1': %2; ").arg(mTestDigit.Char).arg(mTestDigit.Quality));
//
//#ifndef QT_NO_DEBUG
//    // debug
//    int y = 1;
//    Region<uchar>* dbgRegion = mUin->getDebugRegion();
//    if (mDebugDigitPos + dbgRegion->Width() > mObjValue.Width()) {
//      mDebugDigitPos = mObjValue.Width();
//      return;
//    }
//    for (int j = 0; j < dbgRegion->Height(); j++) {
//      const uchar* src = dbgRegion->Line(j);
//      uchar* dst = mObjValue.Data(mDebugDigitPos, j);
//      for (int i = 0; i < dbgRegion->Width(); i++) {
//        *dst++ = *src++;
//      }
//    }
//
//    mDebugDigitPos += dbgRegion->Width() + 2;
//#endif
//  }
}

void UinPre::CalcRegionDiff(const Region<uchar>& regionSrc, Region<uchar>& regionDst)
{
  for (int j = 0; j < regionSrc.Height(); j++) {
    const uchar* src = regionSrc.Line(j);
    uchar* dst = regionDst.Line(j);
    uchar b = *src++;
    for (int i = 0; i < regionSrc.Width() - 1; i++) {
      uchar f = *src;
      *dst = (f >= b)? f - b: b - f;
      b = f;

      src++;
      dst++;
    }
  }
}

void UinPre::CalcRegionHyst(const Region<uchar>& region, Hyst& hyst)
{
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    for (int i = 0; i < region.Width(); i++) {
      hyst.Inc(*src);
      src++;
    }
  }
}

void UinPre::CalcRegionHyst2(const Region<uchar>& region, Hyst& hyst)
{
  mVertMass.resize(0);
  mVertMass.resize(region.Width(), 0);
  mHorzMass.resize(0);
  mHorzMass.resize(region.Height(), 0);

  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    int* horz = mHorzMass.data() + j;
    int* vert = mVertMass.data();
    for (int i = 0; i < region.Width(); i++) {
      hyst.Inc(*src);
      *horz += *src;
      *vert += *src;

      src++;
      vert++;
    }
  }
}

void UinPre::Mk2Color(const Hyst& hyst, const Region<uchar>& regionSrc, Region<uchar>& regionDst)
{
  int value = hyst.GetMidValue(50, 950);

  for (int j = 0; j < regionSrc.Height(); j++) {
    const uchar* src = regionSrc.Line(j);
    uchar* dst = regionDst.Line(j);
    for (int i = 0; i < regionSrc.Width(); i++) {
      if (*src < value) {
        *dst = 0;
      } else {
        *dst = 255;
      }

      src++;
      dst++;
    }
  }
}

void UinPre::Mk3Color(const Hyst& hyst, const Region<uchar>& regionSrc, Region<uchar>& regionDst)
{
  int low = hyst.GetValue(50);
  int high = hyst.GetValue(950);
  int d = high - low;
  low += d/4;
  high -= d/4;

  for (int j = 0; j < regionSrc.Height(); j++) {
    const uchar* src = regionSrc.Line(j);
    uchar* dst = regionDst.Line(j);
    for (int i = 0; i < regionSrc.Width(); i++) {
      if (*src < low) {
        *dst = 0;
      } else if (*src < high) {
        *dst = 128;
      } else {
        *dst = 255;
      }

      src++;
      dst++;
    }
  }
}

void UinPre::DebugHyst(const Hyst& hyst, Region<uchar>& debugRegion)
{
  QVector<int> values = hyst.GetVector();
  int maxValue = 20;
  for (int i = 0; i < values.size(); i++) {
    maxValue = qMax(maxValue, values.at(i));
  }

  debugRegion.SetSize(256, mObjSource.Height() + 1 + values.size());
  debugRegion.FillData(0x7f7f7f7f);
  for (int i = 0; i < values.size(); i++) {
    int v = values.at(i) * 255 / maxValue;
    uchar* dst = debugRegion.Line(mObjSource.Height() + 1 + i);
    for (int j = 0; j < v; j++) {
      *dst++ = (uchar)255;
    }
  }

  for (int j = 0; j < mObjSource.Height(); j++) {
    const uchar* src = mObjSource.Line(j);
    uchar* dst = debugRegion.Line(j);
    for (int i = 0; i < qMin(mObjSource.Width(), debugRegion.Width()); i++) {
      *dst++ = *src++;
    }
  }
  uchar* dst = debugRegion.Line(mObjSource.Height());
  for (int i = 0; i < debugRegion.Width(); i++) {
    *dst++ = (uchar)0x80;
  }
}

bool UinPre::IsValidObj(const UinPre::Obj& obj)
{
  if (obj.Dimentions.Height() < 8) {
    return false;
  }
  qreal k = (qreal)obj.Dimentions.Width() / obj.Dimentions.Height();
  return k > 4 && k < 10;
}

int UinPre::Median(int a, int b, int c)
{
  if (a > b) {
    if (b > c) {
      return b;
    } else {
      return (a < c)? a: c;
    }
  } else {
    if (a > c) {
      return a;
    } else {
      return (b < c)? b: c;
    }
  }
}


UinPre::UinPre()
  : mTopThreshold(kTopThresholdDefault)
{
}

