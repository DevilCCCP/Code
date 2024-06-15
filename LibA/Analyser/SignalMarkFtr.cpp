#include "SignalMarkFtr.h"


using namespace SignalCtor;

const int kSignalValueMin = 10;
const int kSignalWidthMax = 16;

QString SignalMarkFtr::LineNameExtrem()
{
  return "Extrem";
}

QString SignalMarkFtr::LineNameMove()
{
  return "Move";
}

QString SignalMarkFtr::LineNameSignal()
{
  return "Signal";
}

bool SignalMarkFtr::LineTestExtrem(QVector<uchar>& mark)
{
  LineCalc();

  mark.fill(0, mSrcLineSize);
  foreach (const Extrem& exm, mLineExm) {
    mark[exm.Location] = exm.IsMaxi? 100: 1;
  }

  return true;
}

bool SignalMarkFtr::LineTestMove(QVector<uchar>& mark)
{
  LineCalc();

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

  return true;
}

bool SignalMarkFtr::LineTestSignal(QVector<uchar>& mark)
{
  LineCalc();

  mark.fill(0, mSrcLineSize);
  foreach (const Signal& sig, *mCurrentSignalLine) {
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

  return true;
}

void SignalMarkFtr::RegionInfoSignalRaw(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Signal raw";

  filterInfo->Param1Name    = "min value";
  filterInfo->Param1Min     = 4;
  filterInfo->Param1Max     = 40;
  filterInfo->Param1Default = 10;

  filterInfo->Param2Name    = "max width";
  filterInfo->Param2Min     = 4;
  filterInfo->Param2Max     = 80;
  filterInfo->Param2Default = 16;
}

void SignalMarkFtr::RegionInfoSignalLevel(FilterInfo* filterInfo)
{
  filterInfo->Name          = "Signal level";

  filterInfo->Param1Name    = "avr. level";
  filterInfo->Param1Min     = 0;
  filterInfo->Param1Max     = 255;
  filterInfo->Param1Default = 180;

  filterInfo->Param2Name    = "diff level";
  filterInfo->Param2Min     = 0;
  filterInfo->Param2Max     = 255;
  filterInfo->Param2Default = 40;
}

bool SignalMarkFtr::RegionTestSignalRaw(int minV, int minD)
{
  mSignalValueMin  = minV;
  mSignalWidthMax  = minD;

  ClearPrimary();
  CalcSignal();

  ByteRegion* debug = GetAnalyser()->PrepareResultWhite();
  for (int j = 0; j < Source().Height(); j++) {
    SignalLine* lineSig = &mSignalArea[j];
    foreach (const Signal& sig, *lineSig) {
      int value = -sig.TopValue;
      uchar* dbg = debug->Data(sig.Left, j);
      for (int i = sig.Left; i <= sig.Right; i++) {
        *dbg = value;

        dbg++;
      }
    }
  }

  return true;
}

bool SignalMarkFtr::RegionTestSignalLevel(int level, int levelD)
{
  CalcSignal();

  ByteRegion* debug = GetAnalyser()->PrepareResultWhite();
  for (int j = 0; j < Source().Height(); j++) {
    SignalLine* lineSig = &mSignalArea[j];
    foreach (const Signal& sig, *lineSig) {
      int value = -sig.TopValue;
      int diff = qAbs(value - level);
      if (diff <= levelD) {
        uchar* dbg = debug->Data(sig.Left, j);
        for (int i = sig.Left; i <= sig.Right; i++) {
          *dbg = value;

          dbg++;
        }
      }
    }
  }

  return true;
}

void SignalMarkFtr::LineCalc()
{
  mSignalArea.clear();
  mSignalArea.resize(1);
  mCurrentSignalLine = &mSignalArea.front();

  mSrcLine     = Line();
  mSrcLineSize = LineSize();

  MarkLine();
}

SignalArea* SignalMarkFtr::CalcSignal()
{
  if (!StartCalcPrimary()) {
    return &mSignalArea;
  }

  mSignalArea.clear();
  mSignalArea.resize(Source().Height());
  if (Source().Width() < 2) {
    ClearPrimary();
    return nullptr;
  }

  mSrcLineSize = Source().Width();
  for (int j = 0; j < Source().Height(); j++) {
    mSrcLine            = Source().Line(j);
    mCurrentSignalLine  = &mSignalArea[j];
    MarkLine();
  }
  return &mSignalArea;
}

const SignalArea& SignalMarkFtr::GetSignalArea() const
{
  return mSignalArea;
}

void SignalMarkFtr::MarkLine()
{
  if (mSrcLineSize < 2) {
    return;
  }

  mLineExm.clear();
  MarkLineExtrem();
  MarkLineMove();
  MarkLineSignal();
  MarkLineSignalOptimizeAndFilter();
}

enum EExmState {
  eNoExm,
  eMini,
  eMaxi,
};

void SignalMarkFtr::MarkLineExtrem()
{
  Q_ASSERT(mSrcLineSize >= 2);

  EExmState exmState = eNoExm;

  int first = -(int)mSrcLine[0];
  int secnd = -(int)mSrcLine[1];
  if (first < secnd) {
    mLineExm.append(Extrem());
    mLineExm.last().IsMaxi = false;
    mLineExm.last().Location = 0;
    mLineExm.last().Value = first;

    exmState = eMaxi;
    mMinValue = first;
    mMaxValue = secnd;
  } else if (first > secnd) {
    mLineExm.append(Extrem());
    mLineExm.last().IsMaxi = true;
    mLineExm.last().Location = 0;
    mLineExm.last().Value = first;

    exmState = eMini;
    mMaxValue = first;
    mMinValue = secnd;
  } else {
    mMaxValue = mMinValue = first;
    exmState = eNoExm;
  }

  for (int i = 2; i < mSrcLineSize; i++) {
    int next = -(int)mSrcLine[i];
    switch (exmState) {
    case eNoExm:
      if (next < mMinValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = true;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = first;

        exmState = eMini;
        mMinValue = first;
        mMaxValue = secnd;
        i--;
      } else if (next > mMinValue) {
        mLineExm.append(Extrem());
        mLineExm.last().IsMaxi = false;
        mLineExm.last().Location = i - 1;
        mLineExm.last().Value = first;

        exmState = eMaxi;
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
        exmState = eMaxi;
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
        exmState = eMini;
        mMinValue = next;
      } else {
        mMaxValue = next;
      }
      break;
    }
  }
  mLineExm.append(Extrem());
  mLineExm.last().IsMaxi   = exmState == eMaxi;
  mLineExm.last().Location = mSrcLineSize - 1;
  mLineExm.last().Value    = exmState == eMaxi? mMaxValue: mMinValue;
}

void SignalMarkFtr::MarkLineMove()
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

enum ESigState {
  eNoSig,
  eLb,
  eLt,
  eRt,
  eRb,
  eRb2,
};

void SignalMarkFtr::MarkLineSignal()
{
  if (mLineExm.size() < 2) {
    return;
  }

  mLostSignal = nullptr;
  mCurrentSignalLine->append(Signal());
  ESigState sigState = eLb;
  for (auto itr = mLineMov.begin(); itr != mLineMov.end(); ++itr) {
    const Move& curMov = *itr;
    switch (sigState) {
    case eNoSig:
    case eLb:
      if (curMov.Rise > 0) {
        sigState = eLt;
        mCurrentSignalLine->last().Left      = curMov.Left;
        mCurrentSignalLine->last().LeftValue = curMov.LeftValue;
        mCurrentSignalLine->last().Top       = curMov.Right;
        mCurrentSignalLine->last().TopValue  = curMov.RightValue;
      }
      break;

    case eLt:
      if (curMov.Rise > 0) {
//        MarkLineSignalEnd();
//        mCurrentSignalLine->last().Left      = curMov.Left;
//        mCurrentSignalLine->last().LeftValue = curMov.LeftValue;
//        mCurrentSignalLine->last().Top       = curMov.Right;
//        mCurrentSignalLine->last().TopValue  = curMov.RightValue;

        if (curMov.RightValue > mCurrentSignalLine->last().TopValue) {
          mCurrentSignalLine->last().Top       = curMov.Right;
          mCurrentSignalLine->last().TopValue  = curMov.RightValue;
        }
      } else if (curMov.Rise < 0) {
        sigState = eRb;
        if (curMov.LeftValue > mCurrentSignalLine->last().TopValue) {
          mCurrentSignalLine->last().Top       = curMov.Left;
          mCurrentSignalLine->last().TopValue  = curMov.LeftValue;
        }
        mCurrentSignalLine->last().Right      = curMov.Right;
        mCurrentSignalLine->last().RightValue = curMov.RightValue;
      } else {
        sigState = eRt;
      }
      break;

    case eRt:
      if (curMov.Rise > 0) {
        sigState = eLt;
        if (curMov.RightValue > mCurrentSignalLine->last().TopValue) {
          mCurrentSignalLine->last().Top       = curMov.Right;
          mCurrentSignalLine->last().TopValue  = curMov.RightValue;
        }
      } else if (curMov.Rise < 0) {
        sigState = eRb;
        if (curMov.LeftValue > mCurrentSignalLine->last().TopValue) {
          mCurrentSignalLine->last().Top       = curMov.Left;
          mCurrentSignalLine->last().TopValue  = curMov.LeftValue;
        }
        mCurrentSignalLine->last().Right      = curMov.Right;
        mCurrentSignalLine->last().RightValue = curMov.RightValue;
      } else {
        ;
      }
      break;

    case eRb:
      if (curMov.Rise > 0) {
        MarkLineSignalEnd();
        sigState = eLt;
        mCurrentSignalLine->last().Left      = curMov.Left;
        mCurrentSignalLine->last().LeftValue = curMov.LeftValue;
        mCurrentSignalLine->last().Top       = curMov.Right;
        mCurrentSignalLine->last().TopValue  = curMov.RightValue;
      } else if (curMov.Rise < 0) {
        mCurrentSignalLine->last().Right      = curMov.Right;
        mCurrentSignalLine->last().RightValue = curMov.RightValue;
      } else {
        sigState = eRb2;
      }
      break;

    case eRb2:
      if (curMov.Rise > 0) {
        MarkLineSignalEnd();
        sigState = eLt;
        mCurrentSignalLine->last().Left      = curMov.Left;
        mCurrentSignalLine->last().LeftValue = curMov.LeftValue;
        mCurrentSignalLine->last().Top       = curMov.Right;
        mCurrentSignalLine->last().TopValue  = curMov.RightValue;
      } else if (curMov.Rise < 0) {
        sigState = eRb;
        mCurrentSignalLine->last().Right      = curMov.Right;
        mCurrentSignalLine->last().RightValue = curMov.RightValue;
      } else {
        ;
      }
      break;
    }
  }

  if (sigState >= eRb) {
    MarkLineSignalEnd();
  }
  mCurrentSignalLine->removeLast();
  if (mLostSignal) {
    mCurrentSignalLine->removeLast();
  }
}

void SignalMarkFtr::MarkLineSignalEnd()
{
  mCurrentSignalLine->append(Signal());

//  /// case of M signal, glue 2 parts of M or remove part
//  if (mLostSignal) {
//    /// last signal is 'lost' (left M)
//    if (mSignalBallance * (curSig.TopValue - curSig.LeftValue) < (curSig.TopValue - curSig.RightValue)) {
//      /// right M signal - glue
//      if (mLostSignal->TopValue < curSig.TopValue) {
//        mLostSignal->TopValue = curSig.TopValue;
//        mLostSignal->Top      = curSig.Top;
//      }
//      mLostSignal->RightValue = curSig.RightValue;
//      mLostSignal->Right      = curSig.Right;
//      mLostSignal = nullptr;
//    } else if (mSignalBallance * (curSig.TopValue - curSig.RightValue) < (curSig.TopValue - curSig.LeftValue)) {
//      /// left M signal - glue to 'lost'
//      if (mLostSignal->TopValue < curSig.TopValue) {
//        mLostSignal->TopValue = curSig.TopValue;
//        mLostSignal->Top      = curSig.Top;
//      }
//      mLostSignal->RightValue = curSig.RightValue;
//      mLostSignal->Right      = curSig.Right;
//    } else {
//      /// normal signal - replacing 'lost'
//      *mLostSignal = mCurrentSignalLine->last();
//      mLostSignal = nullptr;
//    }
//  } else {
//    if (mSignalBallance * (curSig.TopValue - curSig.RightValue) < (curSig.TopValue - curSig.LeftValue)) {
//      /// left M signal - mark as 'lost'
//      mCurrentSignalLine->append(Signal());
//      mLostSignal = &mCurrentSignalLine->last() - 1;
//    } else if (mSignalBallance * (curSig.TopValue - curSig.LeftValue) < (curSig.TopValue - curSig.RightValue)) {
//      /// right M signal - remove
//    } else {
//      /// normal signal - adding
//      mCurrentSignalLine->append(Signal());
//    }
//  }
}

void SignalMarkFtr::MarkLineSignalOptimizeAndFilter()
{
  for (auto itr = mCurrentSignalLine->begin(); itr != mCurrentSignalLine->end(); ) {
    Signal* signal = itr;
    int minValue  = (-(signal->TopValue + qMax(signal->LeftValue, signal->RightValue))) / 2;
    const uchar* src = &mSrcLine[signal->Left];
    for (int i = signal->Left; i <= signal->Right; i++) {
      if (*src <= minValue) {
        signal->Left = i;
        break;
      }
      src++;
    }
    src = &mSrcLine[signal->Right];
    for (int i = signal->Right; i >= signal->Left; i--) {
      if (*src <= minValue) {
        signal->Right = i;
        break;
      }
      src--;
    }

    if (signal->Right - signal->Left + 1 <= mSignalWidthMax) {
      ++itr;
    } else {
      itr = mCurrentSignalLine->erase(itr);
    }
  }
}


SignalMarkFtr::SignalMarkFtr(Analyser* _Analyser)
  : ImageFilter(_Analyser)
{
  mSignalValueMin  = kSignalValueMin;
  mSignalWidthMax  = kSignalWidthMax;
}
