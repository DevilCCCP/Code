//#include <QDebug>

#include "SignalMark2.h"


const int kSignalThreshold = 5;
const int kSignalHeightMin = 6;
const int kSignalHeightMax = 30;
const int kHorzContextLength = 60;
const int kVertContextLength = 6;
const int kSignalContextCount = 4;

void SignalMark2::Calc(const Region<uchar>* region)
{
  mSrc = region;
  mRegionSig.clear();
  mRegionSig.resize(region->Height());
  mRegionSigBad.clear();
  mRegionSigBad.resize(region->Height());

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentSig = &mRegionSig[j];
    const uchar* src = mSrc->Line(j);
    MarkLine(src, mSrc->Width());
  }

//  for (int j = 0; j < mSrc->Height(); j++) {
//    mCurrentSig    = &mRegionSig[j];
//    FilterSignalWidth(3, 12);
//    FilterSignalMaxValue(70);
//  }

//  ContextFilter(20, 4, 6);

//  mSignalBreakThreshold = -1;
//  for (int j = 1; j < mSrc->Height(); j++) {
//    LineSig* line1 = &mRegionSig[j - 1];
//    LineSig* line2 = &mRegionSig[j];
//    MergeLine2(line1, line2);
//  }

//  for (int j = mSrc->Height() - 1; j > 0; j--) {
//    LineSig* line1 = &mRegionSig[j];
//    LineSig* line2 = &mRegionSig[j - 1];
//    EqualLineContinue(line1, line2);
//  }

//  for (int j = 0; j < mSrc->Height(); j++) {
////    int j = 0;
//    mCurrentSig    = &mRegionSig[j];
//    FilterSignalWidth(2, 12);
//    FilterSignalContinue(2, 26);
//  }

//  mRegionCon.clear();
//  mRegionCon.resize(region->Height());
//  mCurrentArea.clear();
//  mAllArea.clear();
//  for (int j = 0; j < mSrc->Height(); j++) {
//    mCurrentSig    = &mRegionSig[j];
//    mCurrentSigBad = &mRegionSigBad[j];
//    mCurrentCon    = &mRegionCon[j];
//    FilterSignal();
//    ConnectLine();
//    ConnectAreaLine(j);
//  }
}

void SignalMark2::CalcLine(const uchar* src, int size)
{
  mLineSig.clear();
  mCurrentSig = &mLineSig;

  MarkLine(src, size);
}

void SignalMark2::FillLineMark(uchar* mark, int size)
{
  memset(mark, 0, size);
  foreach (const Signal& signal, mLineSig) {
    for (int i = signal.Left; i <= signal.Right; i++) {
      mark[i] = (uchar)(uint)qMin(255, signal.Power);
    }
  }
}

void SignalMark2::FillRegionMark(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->ZeroData();

  for (int j = mSrc->Height() - 1; j > 0; j--) {
    LineSig* line1 = &mRegionSig[j];
    LineSig* line2 = &mRegionSig[j - 1];
    EqualLine(line1, line2);
  }

  for (int j = 0; j < mSrc->Height(); j++) {
    const LineSig& currentSigLine = mRegionSig.at(j);
    uchar* mark = debug->Line(j);

    foreach (const Signal& signal, currentSigLine) {
      if (signal.Height >= kSignalHeightMin && signal.Height > signal.Width * 3/2) {
        for (int i = signal.Left; i <= signal.Right; i++) {
          mark[i] = (uchar)(uint)qMin(255, signal.Height * 10);
        }
      }
    }
  }
}

void SignalMark2::FillRegionMark2(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->ZeroData();

  for (int j = 0; j < mSrc->Height(); j++) {
    const LineSig& currentSigLine    = mRegionSig.at(j);
    const LineSig& currentSigBadLine = mRegionSigBad.at(j);
    const LineCon& currentConLine    = mRegionCon.at(j);
    uchar* mark = debug->Line(j);

    foreach (const SignalConnect& signalCon, currentConLine) {
      for (int i = signalCon.Left; i <= signalCon.Right; i++) {
        mark[i] = (uchar)30;
      }
    }

    foreach (const Signal& signal, currentSigBadLine) {
      int val = FinalSignalMark(signal);
      for (int i = signal.Left; i <= signal.Right; i++) {
        mark[i] = (uchar)val;
      }
    }
    foreach (const Signal& signal, currentSigLine) {
      int val = FinalSignalMark(signal);
      for (int i = signal.Left; i <= signal.Right; i++) {
        mark[i] = (uchar)val;
      }
    }
  }

  foreach (const SignalArea& signalArea, mAllArea) {
    for (int j = signalArea.Top; j <= signalArea.Bottom; j += signalArea.Bottom - signalArea.Top) {
      uchar* mark = debug->Data(signalArea.Left, j);
      for (int i = 0; i < signalArea.Right - signalArea.Left + 1; i++) {
        *mark = 200;
        mark++;
      }
    }
  }
}

void SignalMark2::FillRegionMark3(Region<uchar>* debug)
{
  debug->SetSize(mSrc->Width(), mSrc->Height());
  debug->ZeroData();

  for (int j = 0; j < mSrc->Height(); j++) {
    const LineSig& currentSigLine = mRegionSig.at(j);
//    const uchar* src = (const uchar*)mSrc->Line(j);
    uchar* mark = debug->Line(j);
    auto itr = currentSigLine.begin();

    int l = 0;
    int r = mSrc->Width() - 1;
    for (; ; itr++) {
      r = (itr != currentSigLine.end())? itr->Left - 1: mSrc->Width() - 1;

//      int v = 0;
      int v = 255;
//      for (int i = l; i <= r; i++) {
//        v = qMax(v, (int)src[i]);
//      }
      for (int i = l; i <= r; i++) {
        mark[i] = v;
      }

      if (itr == currentSigLine.end()) {
        break;
      }

      const Signal& signal = *itr;
//      int val = qMin(255, signal.Continue * 10);
      int val = signal.Power;
      if (signal.Break > 0) {
        val = 200;
      }
      for (int i = signal.Left; i <= signal.Right; i++) {
        mark[i] = (uchar)val;
      }
      l = signal.Right + 1;
    }
  }
}

void SignalMark2::FillRegionLevel(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->ZeroData();

  for (int j = 0; j < mSrc->Height(); j++) {
    const LineSig& currentSigLine = mRegionSig.at(j);
    uchar* mark = debug->Line(j);
    int i = 0;
    LineSig::const_iterator curSignalItr = currentSigLine.constBegin();
    int nextSig = curSignalItr != currentSigLine.constEnd()? curSignalItr->Left: mSrc->Width();

    forever {
      if (i < nextSig) {
        int sum = 0;
        const uchar* src = mSrc->Data(i, j);
        for (int ii = i; ii < nextSig; ii++) {
          sum += *src;
          src++;
        }
        int val = sum / (nextSig - i);
        for (int ii = i; ii < nextSig; ii++) {
          *mark = val;
          mark++;
        }
        i = nextSig;
      }

      if (curSignalItr == currentSigLine.constEnd()) {
        break;
      }

      int val = curSignalItr->Level;
      for (int ii = curSignalItr->Left; ii <= curSignalItr->Right; ii++) {
        *mark = val;
        mark++;
      }
      i = curSignalItr->Right + 1;
      curSignalItr++;
      nextSig = curSignalItr != currentSigLine.constEnd()? curSignalItr->Left: mSrc->Width();
    }
  }
}

void SignalMark2::MarkLine(const uchar* src, int size)
{
  int srcl = -(int)*src;
  src++;

  mState = eL0;
  for (int i = 1; i < size; i++) {
    int srcn = -(int)*src;
    int power = srcn - srcl;
    switch (mState) {
    case eL0:
      if (power > kSignalThreshold) {
        mL1 = i - 1;
        mL1Value = srcl;
        mState = eL1;
      }
      break;

    case eL1:
      if (power < kSignalThreshold) {
        if (power < -kSignalThreshold) {
          mL2 = mR2 = i - 1;
          mTopValue = mL2Value = mR2Value = srcl;
          mState = eR2;
          mPowerThreshold = (mL2Value - mL1Value)/4;
        } else if (srcn - mL1Value < kSignalThreshold * (i - mL1)) {
          mState = eL0;
        } else {
          mL2 = i - 1;
          mTopValue = mL2Value = srcl;
          mState = eL2;
          mPowerThreshold = (mL2Value - mL1Value)/4;
        }
      }
      break;

    case eL2:
      if (i - mL2 > mLengthMax) {
        mState = eL0;
      } else if (power > kSignalThreshold) {
        if (i - mL2 >= 3 || srcn - mL1Value < kSignalThreshold * (i - mL1)) {
          mL12 = i - 1;
          mL12Value = srcl;
          mState = eL12;
        } else {
          mState = eL1;
        }
      } else if (power < -kSignalThreshold) {
        mR2 = i - 1;
        mR2Value = srcl;
        mState = eR2;
      } else {
        mTopValue = qMax(mTopValue, srcn);
      }
      break;

    case eL12:
      if (i - mL2 > mLengthMax) {
        mState = eL0;
      } else if (power > kSignalThreshold) {
        if (srcn - mL1Value < kSignalThreshold * (i - mL1)) {
          if (srcn - mL12Value > mPowerThreshold) {
            mL1 = mL12;
            mL1Value = mL12Value;
            mState = eL1;
          } else {
            mTopValue = qMax(mTopValue, srcn);
          }
        } else {
          mState = eL1;
        }
      } else if (power < -kSignalThreshold) {
        mR2 = i - 1;
        mR2Value = srcl;
        mState = eR2;
      } else if (srcn - mL12Value < kSignalThreshold * (i - mL12)) {
        mState = eL2;
      } else {
        mTopValue = qMax(mTopValue, srcn);
      }
      break;

    case eR2:
      if (power > kSignalThreshold) {
        if (i - mL2 < 3 && mL1Value - srcn > kSignalThreshold * (i - mL1)) {
          mState = eL1;
        } else {
          mR1 = i - 1;
          mR1Value = srcl;
          if (EndSignal()) {
            mL1 = i - 1;
            mL1Value = srcl;
            mState = eL1;
          } else {
            mState = eL2;
          }
        }
      } else if (power > -kSignalThreshold) {
        mR1 = i - 1;
        mR1Value = srcl;
        mState = eR1;
      }
      break;

    case eR1:
      if (power < -kSignalThreshold) {
        if (mR2Value - srcn > kSignalThreshold * (i - mR2)) {
          mR1 = i;
          mR1Value = srcn;
          mState = eR2;
        } else if (mR2Value - srcl <= mPowerThreshold) {
          mR2 = i - 1;
          mR2Value = srcl;
          mState = eR2;
        }
      } else if (power > kSignalThreshold) {
        if (mR2Value - srcl > mPowerThreshold) {
          EndSignal();
          mL1 = i - 1;
          mL1Value = srcl;
          mState = eL1;
        } else {
          mState = eL2;
        }
      } else if (i - mR1 >= 3 && mR2Value - srcl > mPowerThreshold) {
        if (EndSignal()) {
          mState = eL0;
        } else {
          mR1 = i - 1;
          mR1Value = srcl;
          mState = eR1;
        }
      }
      break;
    }

//    qDebug() << i << power << mState << "(" << mL1 << mL2 << mR2 << mR1 << ")";
    srcl = srcn;
    src++;
  }
  if (mState == eR2 || mState == eR1) {
    EndSignal();
  }
}

void SignalMark2::MergeLine(SignalMark2::LineSig* line1, SignalMark2::LineSig* line2)
{
  if (line1->isEmpty() || line2->isEmpty()) {
    return;
  }

  QVector<Signal>::iterator s1  = line1->begin();
  QVector<Signal>::iterator s2  = line2->begin();
  QVector<Signal>::iterator s1e = line1->end();
  QVector<Signal>::iterator s2e = line2->end();
  int height = s1->Height + 1;
  forever {
    if (s1->Right + 1 > s2->Left && s1->Left - 1 < s2->Right) {
      s1->Height = height;
      s2->Height = height;
    }
    if (s1->Right < s2->Right) {
      if (s1 == s1e) {
        return;
      }
      s1++;
      height = s1->Height + 1;
    } else {
      if (s2 == s2e) {
        return;
      }
      s2++;
    }
  }
}

void SignalMark2::MergeLine2(SignalMark2::LineSig* line1, SignalMark2::LineSig* line2)
{
  if (line1->isEmpty()) {
    return;
  }

  QVector<Signal>::iterator s1  = line1->begin();
  QVector<Signal>::iterator s1e = line1->end();
  QVector<Signal>::iterator s2  = line2->begin();
  QVector<Signal>::iterator s2e = line2->end();
  while (s2 != s2e) {
    if (s1->Right + 2 > s2->Left && s1->Left - 2 < s2->Right) {
      if (!s1->Ok) {
        s1->Continue++;
        s1->Ok = true;
      }
      s2->Break    = s2->Continue? qMin(s2->Break, s1->Break): s1->Break;
      s2->Continue = qMax(s2->Continue, s1->Continue);
    }

    if (s1->Right < s2->Right) {
      s1++;
      if (s1 == s1e) {
        break;
      }
    } else {
      s2++;
    }
  }

  s1 = line1->begin();
  s2 = line2->begin();
  for (; s1 != s1e; s1++) {
    if (s1->Ok || s1->Break > mSignalBreakThreshold) {
      continue;
    }

    for (; s2 != line2->end() && s2->Right < s1->Left; s2++) {
    }
    Signal signal = *s1;
    signal.Left--;
    signal.Right++;
    signal.Break++;
    s2 = line2->insert(s2, signal);
    s2++;
  }
}

void SignalMark2::EqualLine(SignalMark2::LineSig* line1, SignalMark2::LineSig* line2)
{
  if (line1->isEmpty() || line2->isEmpty()) {
    return;
  }

  QVector<Signal>::iterator s1  = line1->begin();
  QVector<Signal>::iterator s2  = line2->begin();
  QVector<Signal>::iterator s1e = line1->end();
  QVector<Signal>::iterator s2e = line2->end();
  forever {
    if (s1->Right + 1 > s2->Left && s1->Left - 1 < s2->Right) {
      s2->Height   = s1->Height;
      s2->Continue = s1->Continue;
    }
    if (s1->Right < s2->Right) {
      s1++;
      if (s1 == s1e) {
        return;
      }
    } else {
      s2++;
      if (s2 == s2e) {
        return;
      }
    }
  }
}

void SignalMark2::EqualLineContinue(SignalMark2::LineSig* line1, SignalMark2::LineSig* line2)
{
  if (line1->isEmpty() || line2->isEmpty()) {
    return;
  }

  QVector<Signal>::iterator s1  = line1->begin();
  QVector<Signal>::iterator s2  = line2->begin();
  QVector<Signal>::iterator s1e = line1->end();
  QVector<Signal>::iterator s2e = line2->end();
  forever {
    if (s1->Right + 1 > s2->Left && s1->Left - 1 < s2->Right) {
      s2->Continue = s1->Continue;
      s2->Ok = s1->Ok;
    }
    if (s1->Right < s2->Right) {
      s1++;
      if (s1 == s1e) {
        return;
      }
    } else {
      s2++;
      if (s2 == s2e) {
        return;
      }
    }
  }
}

void SignalMark2::FilterSignal()
{
  for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
    if (FinalSignalFilter(*itr)) {
      itr++;
    } else {
      mCurrentSigBad->append(*itr);
      itr = mCurrentSig->erase(itr);
    }
  }
}

void SignalMark2::FilterSignalMaxValue(int maxValue)
{
  for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
    if (itr->Value <= maxValue) {
      itr++;
    } else {
      itr = mCurrentSig->erase(itr);
    }
  }
}

void SignalMark2::FilterSignalWidth(int minWidth, int maxWidth)
{
  for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
    int width = itr->Right - itr->Left + 1;
    if (width >= minWidth && width <= maxWidth) {
      itr++;
    } else {
      itr = mCurrentSig->erase(itr);
    }
  }
}

void SignalMark2::FilterSignalContinue(int minValue, int maxValue)
{
  for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
    if (itr->Continue >= minValue && itr->Continue <= maxValue) {
      itr++;
    } else {
      itr->Level = 200;
      itr++;
//      itr = mCurrentSig->erase(itr);
    }
  }
}

void SignalMark2::ConnectLine()
{
  if (mCurrentSig->isEmpty()) {
    return;
  }

  auto leftItr = mCurrentSig->begin();
  auto rightItr = mCurrentSig->begin();
  int width = 2;
  SignalConnect* currentSigCon = nullptr;
  for (rightItr++; rightItr != mCurrentSig->end(); rightItr++) {
    if (rightItr->Right - leftItr->Left > kHorzContextLength) {
      for (; leftItr != rightItr && rightItr->Right - leftItr->Left > kHorzContextLength; leftItr++) {
        --width;
      }
    }
    if (width >= kSignalContextCount) {
      if (!currentSigCon || leftItr->Left > currentSigCon->Right) {
        mCurrentCon->append(SignalConnect());
        currentSigCon = &mCurrentCon->last();
        currentSigCon->Left  = leftItr->Left;
      }
      currentSigCon->Right = rightItr->Right;
    }
    ++width;
  }
}

void SignalMark2::ConnectAreaLine(int j)
{
  QVector<SignalArea> lastArea;
  mCurrentArea.swap(lastArea);
  auto sigItr = mCurrentCon->begin();
  for (auto areaItr = lastArea.begin(); ; ) {
    if (areaItr == lastArea.end()) {
      for (; sigItr != mCurrentCon->end(); sigItr++) {
        SignalArea area;
        area.Left   = sigItr->Left;
        area.Right  = sigItr->Right;
        area.Top    = j;
        area.Bottom = j;
        mCurrentArea.append(area);
      }
      return;
    } else if (sigItr == mCurrentCon->end()) {
      for (; areaItr != lastArea.end(); areaItr++) {
        if (areaItr->Bottom - areaItr->Top >= kVertContextLength) {
          mAllArea.append(*areaItr);
        }
      }
      return;
    }
    int intrLeft  = qMax(sigItr->Left, areaItr->Left);
    int intrRight = qMin(sigItr->Right, areaItr->Right);
    if (intrRight - intrLeft >= kHorzContextLength) {
      areaItr->Left   = intrLeft;
      areaItr->Right  = intrRight;
      areaItr->Bottom = j;
      areaItr++;
      sigItr++;
    } else if (areaItr->Right < sigItr->Right) {
      if (areaItr->Bottom - areaItr->Top >= kVertContextLength) {
        mAllArea.append(*areaItr);
      }
      areaItr++;
    } else {
      sigItr++;
    }
  }
}

void SignalMark2::ContextFilter(int width, int height, int threshold)
{
  mSignalContext.SetSize((mSrc->Width() - 1)/width + 1, (mSrc->Height() - 1)/height + 1);
  mSignalContext.ZeroData();
  for (int j = 0; j < mSrc->Height(); j++) {
    int segJ = j / height;
    mCurrentSig    = &mRegionSig[j];
    for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); itr++) {
      const Signal& signal = *itr;
      int segI1 = signal.Left / width;
      int segI2 = signal.Right / width;
      for (int segI = segI1; segI <= segI2; segI++) {
        ++(*mSignalContext.Data(segI, segJ));
      }
    }
  }

  Region<int> signalContext;
  signalContext.SetSize(mSignalContext.Width(), mSignalContext.Height());
  signalContext.ZeroData();
  const int kContext = 6;
  const int kContextMin = 3;
  for (int j = 0; j < mSignalContext.Height(); j++) {
    int contextCount = 0;
    for (int i = 0; i < kContext - 1; i++) {
      if (*mSignalContext.Data(i, j) >= threshold) {
        ++contextCount;
      }
    }
    for (int i = 0; i < mSignalContext.Width() - kContext; i++) {
      if (*mSignalContext.Data(i + kContext - 1, j) >= threshold) {
        ++contextCount;
      }

      if (contextCount >= kContextMin) {
        for (int ii = i; ii < i + kContext; ii++) {
          *signalContext.Data(ii, j) = 1;
        }
      }

      if (*mSignalContext.Data(i, j) >= threshold) {
        --contextCount;
      }
    }
  }

  for (int j = 0; j < mSignalContext.Height() - 1; j++) {
    for (int i = 0; i < mSignalContext.Width(); i++) {
      if (*signalContext.Data(i, j) > 0 && *signalContext.Data(i, j + 1) > 0) {
        *signalContext.Data(i, j) = 2;
        *signalContext.Data(i, j + 1) = 2;
      }
    }
  }

  for (int j = 0; j < mSrc->Height(); j++) {
    int segJ = j / height;
    mCurrentSig    = &mRegionSig[j];
    for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
      const Signal& signal = *itr;
      int segI1 = signal.Left / width;
      int segI2 = signal.Right / width;
      bool ok = false;
      for (int segI = segI1; segI <= segI2; segI++) {
        if (*mSignalContext.Data(segI, segJ) >= threshold && *signalContext.Data(segI, segJ) >= 2) {
          ok = true;
          break;
        }
      }
      if (ok) {
        itr++;
      } else {
        itr = mCurrentSig->erase(itr);
      }
    }
  }
}

bool SignalMark2::EndSignal()
{
  int sl = mL2Value - mL1Value;
  int sr = mR2Value - mR1Value;
  if (sl > 4 * sr || sr > 4 * sl) {
    return false;
  }

  if (/*mR2 - mL2 < 10 && */sl + sr > 40) {
    Signal signal;
    signal.Left     = (mL1 + mL2 + 1)/2;
    signal.Right    = (mR1 + mR2)/2;
    signal.Value    = -(mL2Value + mR2Value)/2;
    signal.Power    = (sl + sr)/2;
    signal.Level    = -mTopValue;
    signal.Continue = 1;
    signal.Break    = 0;
    signal.Width    = mR2 - mL2 + 1;
    signal.Height   = 1;
    signal.Connect  = 0;
    signal.Ok       = false;
    mCurrentSig->append(signal);
  }
  return true;
}

bool SignalMark2::FinalSignalFilter(const SignalMark2::Signal& signal)
{
  Q_UNUSED(signal);
  return true;
//  return signal.Continue >= kSignalHeightMin && signal.Continue <= kSignalHeightMax;
}

int SignalMark2::FinalSignalMark(const SignalMark2::Signal& signal)
{
  return qMin(signal.Power * 4, 255);
//  if (signal.Continue > signal.Width) {
//    if (signal.Continue >= kSignalHeightMin && signal.Continue <= kSignalHeightMax) {
//      return 255;
//    } else if (!signal.Break) {
//      return 100;
//    } else {
//      return 25;
//    }
//  }
//  return 0;
}

int SignalMark2::FinalSignalGood(const SignalMark2::Signal& signal)
{
  return signal.Connect >= kSignalContextCount;
}


SignalMark2::SignalMark2(int _LengthMax)
  : mSrc(nullptr), mLengthMax(_LengthMax)
{
}

