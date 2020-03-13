#include <QDebug>

#include "SignalMark3.h"


const int kSignalTopMin = 80;
const int kSignalValueMin = 10;
const int kSignalWidthMax = 16;
const int kSignalHeightMin = 8;
const int kSignalHeightMax = 28;
const int kPackWidth = 100;
const int kPackMinSpace = 40;
const int kPackMinCount = 5;
const int kAreaMinHeight = 2;

void SignalMark3::RegionCalc1(const Region<uchar>* region)
{
  mSrc = region;
  mRegionSig.clear();
  mRegionSig.resize(mSrc->Height());
  mRegionSig2.clear();
  mRegionSig2.resize(mSrc->Height());
  mRegionPack.clear();
  mRegionPack.resize(mSrc->Height());
  if (mSrc->Width() < 2) {
    return;
  }

  CalcInit();
  mSrcLineSize = mSrc->Width();
  for (int j = 0; j < mSrc->Height(); j++) {
    mSrcLine         = mSrc->Line(j);
    mCurrentLineSig  = &mRegionSig[j];
    MarkLine();
  }
}

void SignalMark3::RegionCalc2()
{
  for (int j = 0; j < mSrc->Height(); j++) {
    FilterLine(j);
    if (j > 0) {
      MergeLine(j);
    }
  }
  for (int j = mSrc->Height() - 1; j > 0; j--) {
    ApplyLine(j);
  }
  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLineSig  = &mRegionSig2[j];
    mCurrentLinePack = &mRegionPack[j];
    MarkLinePack();
  }
  CollectArea();
  FilterArea();
}

const QVector<QRect>& SignalMark3::ResultAreas()
{
  return mRegionArea;
}

void SignalMark3::CalcLine(const uchar* src, int size)
{
  mRegionSig.clear();
  mRegionSig.resize(1);
  mRegionSig2.clear();
  mRegionSig2.resize(1);
  mRegionPack.clear();
  mRegionPack.resize(1);
  mCurrentLineSig = &mRegionSig.front();
  mCurrentLinePack = &mRegionPack.front();

  CalcInit();
  mSrcLine     = src;
  mSrcLineSize = size;
  MarkLine();
}

void SignalMark3::DumpLineExtrem(QVector<uchar>& mark)
{
  mark.fill(0, mSrcLineSize);
  foreach (const Extrem& exm, mLineExm) {
    mark[exm.Location] = exm.IsMaxi? 100: 1;
  }
//  foreach (const Signal& signal, *mCurrentLineSig) {
//    for (int i = signal.Left; i <= signal.Right; i++) {
//      mark[i] = (uchar)(uint)(signal.TopValue - (signal.LeftValue + signal.RightValue)/2);
//    }
  //  }
}

void SignalMark3::DumpLinePhases(QVector<uchar>& mark)
{
  mark.fill(0, mSrcLineSize);
}

void SignalMark3::DumpLineMove(QVector<uchar>& mark)
{
  mark.fill(0, mSrcLineSize);
  foreach (const Move& mov, mLineMov) {
    int length = mov.Right - mov.Left + 1;
    if (mov.Rise > 0) {
      for (int i = mov.Left; i <= mov.Right; i++) {
        mark[i] = (uchar)(uint)((i - mov.Left + 1) * 100 / length);
      }
    } else if (mov.Rise < 0) {
      for (int i = mov.Left; i <= mov.Right; i++) {
        mark[i] = (uchar)(uint)((mov.Right - i + 1) * 100 / length);
      }
    }
//    qDebug() << mov.Left << mov.Right << mov.Rise;
  }
}

void SignalMark3::DumpLineSignal(QVector<uchar>& mark)
{
  mark.fill(0, mSrcLineSize);
  foreach (const Signal& sig, *mCurrentLineSig) {
    int length = sig.Top - sig.Left + 1;
    for (int i = sig.Left; i <= sig.Top; i++) {
      mark[i] = (uchar)(uint)((i - sig.Left + 1) * 100 / length);
    }
    length = sig.Right - sig.Top + 1;
    for (int i = sig.Top; i <= sig.Right; i++) {
      mark[i] = (uchar)(uint)((sig.Right - i + 1) * 100 / length);
    }
//    qDebug() << sig.Left << sig.Top << sig.Right;
  }
}

void SignalMark3::DumpLinePack(QVector<uchar>& mark)
{
  mark.fill(0, mSrcLineSize);
  foreach (const SignalPack& pack, *mCurrentLinePack) {
    for (int i = pack.Left; i <= pack.Right; i++) {
      mark[i] = 1;
    }
  }
}

void SignalMark3::DumpRegionValue(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->FillData(255);

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLineSig = &mRegionSig[j];
    foreach (const Signal& sig, *mCurrentLineSig) {
      int value = -sig.TopValue;
      uchar* dbg = debug->Data(sig.Left, j);
      for (int i = sig.Left; i <= sig.Right; i++) {
        *dbg = value;

        dbg++;
      }
    }
  }
}

void SignalMark3::DumpRegionHeight(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->FillData(255);

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLineSig = &mRegionSig2[j];
    foreach (const Signal& sig, *mCurrentLineSig) {
      if (sig.Height < mSignalHeightMin || sig.Height > mSignalHeightMax) {
        continue;
      }
      int value = 127 * sig.Height / mSignalHeightMax;
      uchar* dbg = debug->Data(sig.Left, j);
      for (int i = sig.Left; i <= sig.Right; i++) {
        *dbg = value;

        dbg++;
      }
    }
  }
}

void SignalMark3::DumpRegionPack(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->SetData(255);

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLinePack = &mRegionPack[j];
    foreach (const SignalPack& pack, *mCurrentLinePack) {
      uchar* dbg = debug->Data(pack.Left, j);
      for (int i = pack.Left; i <= pack.Right; i++) {
        *dbg = 200;

        dbg++;
      }
    }
  }

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLineSig = &mRegionSig[j];
    foreach (const Signal& sig, *mCurrentLineSig) {
      if (sig.Height < mSignalHeightMin || sig.Height > mSignalHeightMax) {
        continue;
      }
      int value = 127 * sig.Height / mSignalHeightMax;
      uchar* dbg = debug->Data(sig.Left, j);
      for (int i = sig.Left; i <= sig.Right; i++) {
        *dbg = value;

        dbg++;
      }
    }
  }
}

void SignalMark3::DumpRegionArea(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->SetData(255);

  foreach (const QRect& rect, mRegionArea) {
    debug->Copy(*mSrc, rect);
  }
}

void SignalMark3::CalcInit()
{
}

void SignalMark3::MarkLine()
{
  if (mSrcLineSize < 2) {
    return;
  }

  mLineExm.clear();
  MarkLineExtrem();
  MarkLineMove();
  MarkLineSignal();
  MarkLineSignalOptimize();
//  MarkLinePack();
}

void SignalMark3::MarkLineExtrem()
{
  int first = -(int)mSrcLine[0];
  int secnd = -(int)mSrcLine[1];
  if (first < secnd) {
    mLineExm.append(Extrem());
    mLineExm.last().IsMaxi = false;
    mLineExm.last().Location = 0;
    mLineExm.last().Value = first;

    mExmState = eMaxi;
    mMinValue = first;
    mMaxValue = secnd;
  } else if (first > secnd) {
    mLineExm.append(Extrem());
    mLineExm.last().IsMaxi = true;
    mLineExm.last().Location = 0;
    mLineExm.last().Value = first;

    mExmState = eMini;
    mMaxValue = first;
    mMinValue = secnd;
  } else {
    mMaxValue = mMinValue = first;
    mExmState = eNoExm;
  }

  for (int i = 2; i < mSrcLineSize; i++) {
    int next = -(int)mSrcLine[i];
    switch (mExmState) {
    case eNoExm:
      if (next < mMinValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = true;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = first;

        mExmState = eMini;
        mMinValue = first;
        mMaxValue = secnd;
        i--;
      } else if (next > mMinValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = false;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = first;

        mExmState = eMaxi;
        mMaxValue = first;
        mMinValue = secnd;
        i--;
      }
      break;

    case eMini:
      if (next > mMinValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = false;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = mMinValue;
        mExmState = eMaxi;
        mMaxValue = next;
      } else {
        mMinValue = next;
      }
      break;

    case eMaxi:
      if (next < mMaxValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = true;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = mMaxValue;
        mExmState = eMini;
        mMinValue = next;
      } else {
        mMaxValue = next;
      }
      break;
    }
  }
  mLineExm.append(Extrem());
  mLineExm.last().IsMaxi   = mExmState == eMaxi;
  mLineExm.last().Location = mSrcLineSize - 1;
  mLineExm.last().Value    = mExmState == eMaxi? mMaxValue: mMinValue;
}

void SignalMark3::MarkLineBackground()
{
  const int kBackgroundLength = 80;

  if (mLineExm.isEmpty()) {
    return;
  }

  QList<Extrem*> history;
  auto itr  = mLineExm.begin();
  history.append(itr);
  for (++itr; itr != mLineExm.end(); ++itr) {
    if (itr->Location > history.first()->Location + kBackgroundLength) {

    }
  }
}

void SignalMark3::MarkLineMove()
{
  mLineMov.clear();
  if (mLineExm.size() < 2) {
    return;
  }

  auto itr  = mLineExm.begin();
  auto itrp = mLineExm.begin();
  for (++itr; itr != mLineExm.end(); ++itr, ++itrp) {
    if (itrp->IsMaxi) {
      if (!itr->IsMaxi && itrp->Value - itr->Value >= mSignalValueMin) {
        mLineMov.append(Move());
        mLineMov.last().Rise       = -1;
        mLineMov.last().Left       = itrp->Location;
        mLineMov.last().LeftValue  = itrp->Value;
        mLineMov.last().Right      = itr->Location;
        mLineMov.last().RightValue = itr->Value;
      }
    } else {
      if (itr->IsMaxi && itr->Value - itrp->Value >= mSignalValueMin) {
        mLineMov.append(Move());
        mLineMov.last().Rise  = 1;
        mLineMov.last().Left       = itrp->Location;
        mLineMov.last().LeftValue  = itrp->Value;
        mLineMov.last().Right      = itr->Location;
        mLineMov.last().RightValue = itr->Value;
      }
    }
  }
}

void SignalMark3::MarkLineSignal()
{
  if (mLineExm.size() < 2) {
    return;
  }

  mLostSignal = nullptr;
  mCurrentLineSig->append(Signal());
  mSigState = eLb;
  for (auto itr = mLineMov.begin(); itr != mLineMov.end(); ++itr) {
    const Move& curMov = *itr;
    switch (mSigState) {
    case eNoSig:
    case eLb:
      if (curMov.Rise > 0) {
        mSigState = eLt;
        mCurrentLineSig->last().Left      = curMov.Left;
        mCurrentLineSig->last().LeftValue = curMov.LeftValue;
        mCurrentLineSig->last().Top       = curMov.Right;
        mCurrentLineSig->last().TopValue  = curMov.RightValue;
      }
      break;

    case eLt:
      if (curMov.Rise > 0) {
        /*if (curMov.Left > mCurrentLineSig->last().Top + kMoveBreakMax) {
          mCurrentLineSig->last().Left      = curMov.Left;
          mCurrentLineSig->last().LeftValue = curMov.LeftValue;
          mCurrentLineSig->last().Top       = curMov.Right;
          mCurrentLineSig->last().TopValue  = curMov.RightValue;
        } else */if (curMov.RightValue > mCurrentLineSig->last().TopValue) {
          mCurrentLineSig->last().Top       = curMov.Right;
          mCurrentLineSig->last().TopValue  = curMov.RightValue;
        }
      } else if (curMov.Rise < 0) {
        mSigState = eRb;
        if (curMov.LeftValue > mCurrentLineSig->last().TopValue) {
          mCurrentLineSig->last().Top       = curMov.Left;
          mCurrentLineSig->last().TopValue  = curMov.LeftValue;
        }
        mCurrentLineSig->last().Right      = curMov.Right;
        mCurrentLineSig->last().RightValue = curMov.RightValue;
      } else {
        mSigState = eRt;
      }
      break;

    case eRt:
      if (curMov.Rise > 0) {
        mSigState = eLt;
        /*if (curMov.Left > mCurrentLineSig->last().Top + kMoveBreakMax) {
          mCurrentLineSig->last().Left      = curMov.Left;
          mCurrentLineSig->last().LeftValue = curMov.LeftValue;
          mCurrentLineSig->last().Top       = curMov.Right;
          mCurrentLineSig->last().TopValue  = curMov.RightValue;
        } else */if (curMov.RightValue > mCurrentLineSig->last().TopValue) {
          mCurrentLineSig->last().Top       = curMov.Right;
          mCurrentLineSig->last().TopValue  = curMov.RightValue;
        }
      } else if (curMov.Rise < 0) {
        mSigState = eRb;
        if (curMov.LeftValue > mCurrentLineSig->last().TopValue) {
          mCurrentLineSig->last().Top       = curMov.Left;
          mCurrentLineSig->last().TopValue  = curMov.LeftValue;
        }
        mCurrentLineSig->last().Right      = curMov.Right;
        mCurrentLineSig->last().RightValue = curMov.RightValue;
      } else {
        ;
      }
      break;

    case eRb:
      if (curMov.Rise > 0) {
        MarkLineSignalEnd();
        mSigState = eLt;
        mCurrentLineSig->last().Left      = curMov.Left;
        mCurrentLineSig->last().LeftValue = curMov.LeftValue;
        mCurrentLineSig->last().Top       = curMov.Right;
        mCurrentLineSig->last().TopValue  = curMov.RightValue;
      } else if (curMov.Rise < 0) {
        /*if (curMov.Left > mCurrentLineSig->last().Right + kMoveBreakMax) {
          MarkLineSignalEnd();
          mSigState = eLb;
        } else*/ {
          mCurrentLineSig->last().Right      = curMov.Right;
          mCurrentLineSig->last().RightValue = curMov.RightValue;
        }
      } else {
        mSigState = eRb2;
      }
      break;

    case eRb2:
      if (curMov.Rise > 0) {
        MarkLineSignalEnd();
        mSigState = eLt;
        mCurrentLineSig->last().Left      = curMov.Left;
        mCurrentLineSig->last().LeftValue = curMov.LeftValue;
        mCurrentLineSig->last().Top       = curMov.Right;
        mCurrentLineSig->last().TopValue  = curMov.RightValue;
      } else if (curMov.Rise < 0) {
        /*if (curMov.Left > mCurrentLineSig->last().Right + kMoveBreakMax) {
          MarkLineSignalEnd();
          mSigState = eLb;
        } else*/ {
          mSigState = eRb;
          mCurrentLineSig->last().Right      = curMov.Right;
          mCurrentLineSig->last().RightValue = curMov.RightValue;
        }
      } else {
        ;
      }
      break;
    }
  }

  if (mSigState >= eRb) {
    MarkLineSignalEnd();
  }
  mCurrentLineSig->removeLast();
  if (mLostSignal) {
    mCurrentLineSig->removeLast();
  }
}

void SignalMark3::MarkLineSignalEnd()
{
  const Signal& curSig = mCurrentLineSig->last();
  if (mLostSignal) {
    if (4*(curSig.TopValue - curSig.LeftValue) < curSig.TopValue - curSig.RightValue) { // glue
      if (mLostSignal->TopValue < curSig.TopValue) {
        mLostSignal->TopValue = curSig.TopValue;
        mLostSignal->Top      = curSig.Top;
      }
      mLostSignal->RightValue = curSig.RightValue;
      mLostSignal->Right      = curSig.Right;
      mLostSignal = nullptr;
    } else if (4*(curSig.TopValue - curSig.RightValue) < curSig.TopValue - curSig.LeftValue) { // replace lost
      *mLostSignal = mCurrentLineSig->last();
    } else { // remove lost
      *mLostSignal = mCurrentLineSig->last();
      mLostSignal = nullptr;
    }
  } else {
    if (4*(curSig.TopValue - curSig.RightValue) < curSig.TopValue - curSig.LeftValue) {
      mCurrentLineSig->append(Signal());
      mLostSignal = &mCurrentLineSig->last() - 1;
    } else if (4*(curSig.TopValue - curSig.LeftValue) < curSig.TopValue - curSig.RightValue) {
      ;
    } else {
      mCurrentLineSig->append(Signal());
    }
  }
}

void SignalMark3::MarkLineSignalOptimize()
{
  for (auto itr = mCurrentLineSig->begin(); itr != mCurrentLineSig->end(); itr++) {
    Signal* signal = itr;
    signal->MinValue  = (-(signal->TopValue + qMax(signal->LeftValue, signal->RightValue))) / 2;
    const uchar* src = &mSrcLine[signal->Left];
    for (int i = signal->Left; i <= signal->Right; i++) {
      if (*src <= signal->MinValue) {
        signal->Left = i;
        break;
      }
      src++;
    }
    src = &mSrcLine[signal->Right];
    for (int i = signal->Right; i >= signal->Left; i--) {
      if (*src <= signal->MinValue) {
        signal->Right = i;
        break;
      }
      src--;
    }
  }
}

void SignalMark3::MarkLinePack()
{
  MarkLinePackOptimize();

  if (mCurrentLineSig->isEmpty()) {
    return;
  }

  auto itr = mCurrentLineSig->begin();
  auto firstSignal = itr;
  auto lastSignal  = itr;
  SignalPack* currentPack = nullptr;
  for (++itr; itr != mCurrentLineSig->end(); itr++) {
    const Signal* signal = itr;
    if (signal->Left - lastSignal->Right > mPackMinSpace) {
      firstSignal = itr;
    }
    lastSignal = itr;
    if (signal->Right - firstSignal->Left > mPackWidth) {
      while (signal->Right - firstSignal->Left > mPackWidth) {
        Q_ASSERT(firstSignal != lastSignal);
        ++firstSignal;
      }
    }
    if (lastSignal - firstSignal + 1 >= mPackMinCount) {
      if (!currentPack) {
        mCurrentLinePack->append(SignalPack());
        currentPack = &mCurrentLinePack->last();
        currentPack->Left = firstSignal->Left;
      }
      currentPack->Right = lastSignal->Right;
    } else {
      if (currentPack) {
        currentPack = nullptr;
      }
    }
  }
}

void SignalMark3::MarkLinePackOptimize()
{
  LineSig filteredLine;
  for (auto itr = mCurrentLineSig->begin(); itr != mCurrentLineSig->end(); itr++) {
    const Signal* signal = itr;
    if (signal->Height >= mSignalHeightMin && signal->Height <= mSignalHeightMax) {
      filteredLine.append(*signal);
    }
  }
  mCurrentLineSig->swap(filteredLine);
}

void SignalMark3::FilterLine(int j)
{
  LineSig*  currentLine = &mRegionSig[j];
  LineSig* filteredLine = &mRegionSig2[j];

  foreach (const Signal& signal, *currentLine) {
    if (-signal.TopValue <= getSignalTopMin() && signal.Right - signal.Left + 1 <= getSignalWidthMax()) {
      filteredLine->append(signal);
    }
  }
}

void SignalMark3::MergeLine(int j)
{
  LineSig* line1 = &mRegionSig2[j-1];
  LineSig* line2 = &mRegionSig2[j];

  if (line1->isEmpty() || line2->isEmpty()) {
    return;
  }

  auto s1  = line1->begin();
  auto s2  = line2->begin();
  auto s1e = line1->end();
  auto s2e = line2->end();
  forever {
    if (s1->Right >= s2->Left && s1->Left <= s2->Right
        /*&& s1->MinValue >= s2->TopValue && s1->TopValue <= s2->MinValue*/) {
      s2->Height = qMax(s2->Height, s1->Height + 1);
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

void SignalMark3::ApplyLine(int j)
{
  LineSig* line1 = &mRegionSig2[j-1];
  LineSig* line2 = &mRegionSig2[j];

  if (line1->isEmpty() || line2->isEmpty()) {
    return;
  }

  auto s1  = line1->begin();
  auto s2  = line2->begin();
  auto s1e = line1->end();
  auto s2e = line2->end();
  forever {
    if (s1->Right >= s2->Left && s1->Left <= s2->Right) {
      s1->Height = qMax(s1->Height, s2->Height);
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

void SignalMark3::CollectArea()
{
  mRegionArea.clear();
  QVector<int> currentLineAreaIndex;
  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentLinePack = &mRegionPack[j];
    QVector<int> nextLineAreaIndex;
    foreach (const SignalPack& pack, *mCurrentLinePack) {
      int foundIndex = -1;
      QRect* foundRect = nullptr;
      foreach (int index, currentLineAreaIndex) {
        QRect* rect = &mRegionArea[index];
        if (pack.Left <= rect->right() && rect->left() <= pack.Right) {
          rect->setLeft(qMin(rect->left(), pack.Left));
          rect->setRight(qMax(rect->right(), pack.Right));
          rect->setBottom(j);

          if (foundRect) {
            *foundRect = foundRect->united(*rect);
            *rect = QRect();
          } else {
            foundRect = rect;
            foundIndex = index;
          }
        }
      }
      if (!foundRect) {
        mRegionArea.append(QRect(QPoint(pack.Left, j), QPoint(pack.Right, j)));
        foundIndex = mRegionArea.size() - 1;
      }
      nextLineAreaIndex.append(foundIndex);
    }
    currentLineAreaIndex.swap(nextLineAreaIndex);
  }
}

void SignalMark3::FilterArea()
{
  QVector<QRect> filteredRegionArea;
  foreach (const QRect& area, mRegionArea) {
    if (area.height() >= getAreaMinHeight()) {
      filteredRegionArea.append(area);
    }
  }
  filteredRegionArea.swap(mRegionArea);
}


SignalMark3::SignalMark3()
  : mSrc(nullptr)
{
  mSignalValueMin  = kSignalValueMin;
  mSignalTopMin    = kSignalTopMin;
  mSignalWidthMax  = kSignalWidthMax;
  mSignalHeightMin = kSignalHeightMin;
  mSignalHeightMax = kSignalHeightMax;
  mPackWidth       = kPackWidth;
  mPackMinSpace    = kPackMinSpace;
  mPackMinCount    = kPackMinCount;
  mAreaMinHeight   = kAreaMinHeight;
}

