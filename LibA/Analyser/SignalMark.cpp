#include "SignalMark.h"


const int kNeytralThreshold = 5;
const int kHorzContextLength = 20;
const int kVertContextLength = 8;

void SignalMark::Calc(const Region<uchar>* region, int contextFilter)
{
  mSrc = region;
  mRegionSig.clear();
  mRegionSig.resize(region->Height());
  mContextLength = contextFilter;

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentSig = &mRegionSig[j];
    const uchar* src = mSrc->Line(j);
    MarkLine(src, mSrc->Width());
    if (mContextLength > 0) {
      ContextFilterLine();
    }
//    if (j > 1) {
//      uchar* dstp = mMark.Line(j - 1);
//      MergeLines(dst, dstp, mSrc->Width());
//    }
  }

//  for (int j = mSrc->Height() - 1; j > 0; j--) {
//    uchar* dst  = mMark.Line(j);
//    uchar* dstp = mMark.Line(j - 1);
//    MergeLinesBack(dst, dstp, mSrc->Width());
//  }

////  SelectLineBegin();
////  for (int j = 0; j < mSrc->Height(); j++) {
////    uchar* dst  = mPreData.Line(j);
////    SelectLine(j, dst, mSrc->Width());
////  }
  //  //  SelectLineEnd();
}

void SignalMark::CalcLine(const uchar* src, int size, int contextFilter)
{
  mLineSig.clear();
  mCurrentSig = &mLineSig;

  MarkLine(src, size);
  if (contextFilter > 0) {
    mContextLength = contextFilter;
    ContextFilterLine();
  }
}

void SignalMark::FillLineMark(uchar* mark, int size)
{
  memset(mark, 0, size);
  foreach (const Signal& signal, mLineSig) {
    for (int i = signal.Left; i <= signal.Right; i++) {
      mark[i] = (uchar)(uint)signal.Power;
    }
  }
}

void SignalMark::FillRegionMark(Region<uchar>* debug)
{
  if (debug->Width() != mSrc->Width() || debug->Height() != mSrc->Height()) {
    debug->SetSize(mSrc->Width(), mSrc->Height());
  }
  debug->ZeroData();

  for (int j = 0; j < mSrc->Height(); j++) {
    mCurrentSig = &mRegionSig[j];
    uchar* mark = debug->Line(j);
    foreach (const Signal& signal, *mCurrentSig) {
      for (int i = signal.Left; i <= signal.Right; i++) {
        mark[i] = (uchar)(uint)signal.Power;
      }
    }
  }
}

void SignalMark::MarkLine(const uchar* src, int size)
{
  uchar srcl = *src;
  src++;
  size--;

  mDirection = eNeytral;
  mSignalPower = kNeytralThreshold;
  for (int i = 0; i < size; i++) {
    int power = (int)*src - (int)srcl;
    if (power <= - mSignalPower) { // up
      if (-power > 4*mSignalPower) {
        mSignalPower = -power/4;
      }
      switch (mDirection) {
      case eNeytral:
        mLeftBottomValue = (int)srcl;
        mDirection = eUp;
        break;
      case eUp:
        break;
      case eTop:
        mDirection = eUp;
        break;
      case eDown:
        mRightBottomValue = (int)srcl;
        [[fallthrough]];
      case eBottom:
        EndSignal();
        mLeftBottomValue = (int)srcl;
        mDirection = eUp;
        break;
      }
    } else if (power >= mSignalPower) { // down
      if (power > 4*mSignalPower) {
        mSignalPower = power/4;
      }
      switch (mDirection) {
      case eNeytral:
        break;
      case eUp:
        mLeftTop = mRightTop = i;
        mLeftTopValue = mRightTopValue = (int)srcl;
        mDirection = eDown;
        break;
      case eTop:
        mRightTop = i;
        mRightTopValue = (int)srcl;
        mDirection = eDown;
        break;
      case eDown:
        break;
      case eBottom:
        mDirection = eDown;
        break;
      }
    } else { // neytral
      switch (mDirection) {
      case eNeytral:
        break;
      case eUp:
        mLeftTop = i;
        mLeftTopValue = (int)srcl;
        mDirection = eTop;
        break;
      case eTop:
        break;
      case eDown:
        mRightBottomValue = (int)srcl;
        break;
      case eBottom:
        break;
      }
    }

    srcl = *src;
    src++;
  }
  if (mDirection == eBottom || mDirection == eDown) {
    EndSignal();
  }
}

void SignalMark::EndSignal()
{
  mSignalPower = kNeytralThreshold;
  if (mRightTop - mLeftTop < mLengthMax) {
    int left  = mLeftBottomValue  - mLeftTopValue;
    int right = mRightBottomValue - mRightTopValue;
    if (left > right) {
      qSwap(left, right);
    }
    if (2 * left > right) {
      Signal signal;
      signal.Left  = mLeftTop;
      signal.Right = mRightTop;
      signal.Power = left;
      mCurrentSig->push_back(signal);
    }
  }
}

void SignalMark::ContextFilterLine()
{
  if (mCurrentSig->empty()) {
    return;
  }
  std::list<LineSig::iterator> remove;
  std::list<int> powerContext;
  auto itrL = mCurrentSig->begin();
  auto itrC = mCurrentSig->begin();
  auto itrR = mCurrentSig->begin();
  powerContext.push_back(itrC->Power);
  for (; itrC != mCurrentSig->end(); itrC++) {
    for (; itrC->Left - itrL->Right > mContextLength; itrL++) {
      if (powerContext.front() == itrL->Power) {
        powerContext.pop_front();
      }
    }
    auto itrRnew = itrR;
    for (++itrRnew; itrRnew != mCurrentSig->end(); itrRnew++) {
      if (itrRnew->Left - itrC->Right > mContextLength) {
        break;
      }
      itrR = itrRnew;
      while (!powerContext.empty() && powerContext.back() < itrR->Power) {
        powerContext.pop_back();
      }
      powerContext.push_back(itrR->Power);
    }

    if (2*itrC->Power < powerContext.front()) {
      remove.push_back(itrC);
    }
  }

  for (auto itr = mCurrentSig->begin(); itr != mCurrentSig->end(); ) {
    if (remove.empty()) {
      break;
    }
    if (itr == remove.front()) {
      itr = mCurrentSig->erase(itr);
      remove.pop_front();
    } else {
      itr++;
    }
  }
}


SignalMark::SignalMark(int _LengthMax)
  : mSrc(nullptr), mLengthMax(_LengthMax)
{
}

