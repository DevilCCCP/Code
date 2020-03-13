#include <Lib/Log/Log.h>

#include "LineObj.h"


const int kBlindTime = 2000;
const int kMinSolidSize = 64;

void LineObj::Init()
{
  mPreObjMark.InitSource();
  mPreObjMarkSolid.InitSource();
  mLayerMark.InitSource();
}

void LineObj::LoadSettings(const SettingsAS& settings)
{
//  mPreMomentHyst.Deserialize(settings->GetValue("PreMomentHyst", QString("")).toString(), 128);
//  mPreMomentHyst2.Deserialize(settings->GetValue("PreMomentHyst2", QString("")).toString(), 128);
}

void LineObj::SaveSettings(const SettingsAS& settings)
{
//  mPreMomentHyst.Normalize(14*24*60*60*30);
//  settings->SetValue("PreMomentHyst", mPreMomentHyst.Serialize(128));
//  mPreMomentHyst2.Normalize(14*24*60*60*30);
//  settings->SetValue("PreMomentHyst2", mPreMomentHyst2.Serialize(128));
}

void LineObj::Analize()
{
  Prepare();
  if (mMarkConnect) {
    MakeMarkMedian();
    ConnectPreObj(mLayerMark);
  } else {
    ConnectPreObj(mLayerF);
  }
  MakePreObj();
  Filter1PreObj();
  MakePreType1();

  if (CurrentMs() > kBlindTime) {
    ObjTakePre();
    ObjApplyPre();
    ObjFromUnusedPre();
  }
//  FillPreObj();
//  BorderPreObj();

//  ConnectLines();
}

qreal LineObj::CalcStable()
{
  return 100.0 * mObj.size();
  return 0;
}

bool LineObj::HaveObj()
{
  for (; mReturnObjItr < mObj.size(); mReturnObjItr++) {
    return true;
  }
  return false;
}

bool LineObj::RetrieveObj(Object& object)
{
  if (!HaveObj()) {
    return false;
  }

  const Obj& obj = mObj[mReturnObjItr++];
  object.Id = mReturnObjItr;
  if (obj.Type == eTest) {
    object.Color = 3100;
  } else if (obj.Type == eCasper) {
    object.Color = 100;
  } else {
    object.Color = 0;
  }
  object.Dimention.Left = obj.Info.Left.x();
  object.Dimention.Right = obj.Info.Right.x();
  object.Dimention.Top = obj.Info.Top.y();
  object.Dimention.Bottom = obj.Info.Bottom.y();
  return true;
}

void LineObj::Prepare()
{
  mReturnObjItr = 0;

  mMinObjLength = qMax(qMin(Width(), Height()) / 25, 4);
  mPreObj.clear();
  mPreObj1.clear();
  mPreObj1Map.clear();

  mPreObjPointRef.clear();
  mObjPrePointRef.clear();
}

void LineObj::MakeMarkSmooth()
{
  mLayerMark.ZeroSource();
  int stride = mLayerF.Stride();
  memcpy(mLayerMark.Line(0), mLayerF.Line(0), stride);
  memcpy(mLayerMark.Line(mLayerF.Height() - 1), mLayerF.Line(mLayerF.Height() - 1), stride);
  for (int j = 1; j < Height() - 1; j++) {
    const uchar* mark1 = mLayerF.Line(j);
    uchar*       mark2 = mLayerMark.Line(j);
    *mark2++ = *mark1++;
    for (int i = 1; i < Width() - 1; i++) {
      if (mark1[0] != 0) {
        mark2[-stride-1] = 1;
        mark2[-stride] = 1;
        mark2[-stride+1] = 1;
        mark2[-1] = 1;
        mark2[0] = 1;
        mark2[1] = 1;
        mark2[stride-1] = 1;
        mark2[stride] = 1;
        mark2[stride+1] = 1;
      }
      mark1++;
      mark2++;
    }
    *mark2 = *mark1;
  }
}

void LineObj::MakeMarkConnect()
{
  int stride = mLayerF.Stride();
  memcpy(mLayerMark.Line(0), mLayerF.Line(0), stride);
  memcpy(mLayerMark.Line(mLayerF.Height() - 1), mLayerF.Line(mLayerF.Height() - 1), stride);
  for (int j = 1; j < Height() - 1; j++) {
    const uchar* mark1 = mLayerF.Line(j);
    uchar*       mark2 = mLayerMark.Line(j);
    *mark2++ = *mark1++;
    for (int i = 1; i < Width() - 1; i++) {
      if (mark1[0] != 0) {
        *mark2 = 1;
      } else if (mark1[-1] != 0 && mark1[1] != 0) {
        *mark2 = 1;
      } else if (mark1[-stride] != 0 && mark1[stride] != 0) {
        *mark2 = 1;
      } else if (mark1[-stride - 1] != 0 && mark1[stride + 1] != 0) {
        *mark2 = 1;
      } else if (mark1[-stride + 1] != 0 && mark1[stride - 1] != 0) {
        *mark2 = 1;
      } else {
        *mark2 = 0;
      }
      mark1++;
      mark2++;
    }
    *mark2 = *mark1;
  }
}

void LineObj::MakeMarkMedian()
{
  int stride = mLayerF.Stride();
  memcpy(mLayerMark.Line(0), mLayerF.Line(0), stride);
  memcpy(mLayerMark.Line(mLayerF.Height() - 1), mLayerF.Line(mLayerF.Height() - 1), stride);
  for (int j = 1; j < Height() - 1; j++) {
    const uchar* mark1 = mLayerF.Line(j);
    uchar*       mark2 = mLayerMark.Line(j);
    *mark2++ = *mark1++;
    for (int i = 1; i < Width() - 1; i++) {
      int count = 0;
      if (mark1[0] != 0) {
        count += 2;
      } if (mark1[-1] != 0) {
        count++;
      } if (mark1[1] != 0) {
        count++;
      } if (mark1[-stride] != 0) {
        count++;
      } if (mark1[stride] != 0) {
        count++;
      } if (mark1[-stride - 1] != 0) {
        count++;
      } if (mark1[stride + 1] != 0) {
        count++;
      } if (mark1[-stride + 1] != 0) {
        count++;
      } if (mark1[stride - 1] != 0) {
        count++;
      }
      *mark2 = (count >= 3)? 1: 0;

      mark1++;
      mark2++;
    }
    *mark2 = *mark1;
  }
}

void LineObj::ConnectPreObj(const ImageSrc<uchar>& layerMark)
{
  int currentObj = 0;
  for (int j = 0; j < Height(); j++) {
    const uchar* mark = layerMark.Line(j);
    int*       object = mPreObjMark.Line(j);

    bool inObject = false;
    for (int i = 0; i < Width(); i++) {
      bool isObject = (*mark != 0);
      if (isObject) {
        if (!inObject) {
          ++currentObj;
          inObject = true;
        }
        *object = currentObj;
      } else {
        if (inObject) {
          inObject = false;
        }
        *object = 0;
      }

      mark++;
      object++;
    }
  }

  mObjIds.fill(0, currentObj + 1);

  for (int j = 0; j < Height() - 1; j++) {
    int* object1 = mPreObjMark.Line(j);
    int* object2 = mPreObjMark.Line(j + 1);

    for (int i = 0; i < Width(); i++) {
      if (*object1 > 0 && *object2 > 0) {
        int obj1 = *object1;
        while (mObjIds[obj1]) {
          obj1 = mObjIds[obj1];
        }
        int obj2 = *object2;
        while (mObjIds[obj2]) {
          obj2 = mObjIds[obj2];
        }
        if (obj1 != obj2) {
          int objNew = qMin(obj1, obj2);
          for (int obj = *object1; obj && obj != objNew; ) {
            int objn = mObjIds[obj];
            mObjIds[obj] = objNew;
            obj = objn;
          }
          for (int obj = *object2; obj && obj != objNew; ) {
            int objn = mObjIds[obj];
            mObjIds[obj] = objNew;
            obj = objn;
          }
        }
      }

      object1++;
      object2++;
    }
  }

  mPreObjCount = 1;
  for (int obj = 1; obj <= currentObj; obj++) {
    if (int objRef = mObjIds[obj]) {
      mObjIds[obj] = mObjIds[objRef];
    } else {
      mObjIds[obj] = mPreObjCount;
      mPreObjCount++;
    }
  }
}

void LineObj::MakePreObj()
{
  mPreObj.resize(mPreObjCount);
  InitVector(mPreObj, mPreObjCount);

  for (int j = 0; j < Height(); j++) {
    int* object = mPreObjMark.Line(j);

    for (int i = 0; i < Width(); i++) {
      if (*object) {
        int obj = mObjIds[*object];
        *object = obj;
        PreObj* objInfo = &mPreObj[obj];
        if (objInfo->Count) {
          objInfo->Dimentions.Left   = qMin(objInfo->Dimentions.Left, i);
          objInfo->Dimentions.Right  = qMax(objInfo->Dimentions.Right, i);
          objInfo->Dimentions.Top    = qMin(objInfo->Dimentions.Top, j);
          objInfo->Dimentions.Bottom = qMax(objInfo->Dimentions.Bottom, j);
        } else {
          objInfo->Dimentions = Rectangle(i, j, i, j);
        }
        objInfo->Count++;
      }

      object++;
    }
  }
}

void LineObj::Filter1PreObj()
{
  mPreObj1.clear();
  mPreObj1Map.resize(mPreObj.size());
  for (int i = 0; i < mPreObj.size(); i++) {
    PreObj* pre = &mPreObj[i];
    mPreObj1Map[i] = -1;
    if (pre->Count < mMinObjLength || qMax(pre->Dimentions.Width(), pre->Dimentions.Height()) < mMinObjLength) {
      continue;
    }
    int w = pre->Dimentions.Width();
    int h = pre->Dimentions.Height();
//    int d = qMax(w, h);
//    int cMax = d*d / 4;
//    if (pre->Count > cMax) {
//      continue;
//    }
    pre->Length0 = w + h;
    if (qMin(w, h) > qMax(w, h)/2) {
      pre->Length1 = qMax(w, h);
      mPreMomentHyst.Inc(pre->Length1);
    } else {
      pre->Length1 = 0;
    }

    mPreObj1Map[i] = mPreObj1.size();
    mPreObj1.append(i);
  }
  mPreObjCount = 0;
}

void LineObj::Mark1PreObj()
{
  mPreObjMarkSolid.ZeroSource();
  for (int i = 0; i < mPreObj1.size(); i++) {
    mCurrentPreIndex = mPreObj1[i];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    Mark1PreObjOne();
  }
}

void LineObj::Mark1PreObjOne()
{
  mLineTmp.SetSize(mCurrentPre->Dimentions.Width(), mCurrentPre->Dimentions.Height());
  QLinkedList<QPoint> nextPoints;
  for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
    const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left, j);
    for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
      int obj = *object;
      if (obj == mCurrentPreIndex) {
        nextPoints.append(QPoint(i - mCurrentPre->Dimentions.Left, j - mCurrentPre->Dimentions.Top));
      }

      object++;
    }
  }

  MarkPointsLength(nextPoints);

  int stride = mLineTmp.Stride();
  for (int j = 1; j < mLineTmp.Height() - 1; j++) {
    const int* tmpnm = mLineTmp.Line(j - 1);
    const int* tmpnn = mLineTmp.Line(j);
    const int* tmpmn = tmpnn - 1;
    for (int i = 1; i < mLineTmp.Width(); i++) {
      if (*tmpnn > *tmpmn && *tmpnn > *tmpnm && *tmpnn >= *(tmpnn + 1) && *tmpnn >= *(tmpnn + stride)) {
        Mark1NewSolid(i, j);
      }

      tmpnn++;
      tmpmn++;
      tmpnm++;
    }
  }
}

void LineObj::Mark1NewSolid(int iCenter, int jCenter)
{
  *mPreObjMark.Data(mCurrentPre->Dimentions.Left + iCenter, mCurrentPre->Dimentions.Top + jCenter) = 666666;
  return;

  int width = mLineTmp.Width();
  int height = mLineTmp.Height();
  int widthLim = width - 1;
  int heightLim = height - 1;

  QLinkedList<QPoint> currentPath;
  QLinkedList<QPoint> solidPoints;
  currentPath.append(QPoint(iCenter, jCenter));
  int colorSum = 0;
  forever {
    QLinkedList<QPoint> nextPath;
    foreach (const QPoint& p, currentPath) {
      int* data = mLineTmp.Data(p);
      int length = *data;
      *data = 0;

      if (length < 4) {
        continue;
      }

      int x = mCurrentPre->Dimentions.Left + p.x();
      int y = mCurrentPre->Dimentions.Top + p.y();
      colorSum += *ImageData().Data(x, y);
      solidPoints.append(QPoint(x, y));

      if (p.x() > 0 && data[-1] <= length) {
        nextPath.append(QPoint(p.x()-1, p.y()));
      }
      if (p.x() < widthLim && data[1] <= length) {
        nextPath.append(QPoint(p.x()+1, p.y()));
      }
      if (p.y() > 0 && data[-width] <= length) {
        nextPath.append(QPoint(p.x(), p.y()-1));
      }
      if (p.y() < heightLim && data[width] <= length) {
        nextPath.append(QPoint(p.x(), p.y()+1));
      }
    }
    if (nextPath.isEmpty()) {
      break;
    }
    currentPath = nextPath;
  }

  if (solidPoints.size() > kMinSolidSize) {
    int midColor = colorSum / solidPoints.size();
    foreach (const QPoint& p, solidPoints) {
      *mPreObjMarkSolid.Data(p.x(), p.y()) = midColor;
    }
  }
}

void LineObj::FillPreObj()
{
  mPreObjMarkSolid.ZeroSource();
  for (int i = 0; i < mPreObj1.size(); i++) {
    mCurrentPreIndex = mPreObj1[i];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    FillPreObjOne();
  }
}

void LineObj::FillPreObjOne()
{
  const int kFillFromBorder = 2;

  for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
    const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left, j);
    int last = mCurrentPre->Dimentions.Right;
    for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
      int obj = *object;
      if (obj == mCurrentPreIndex) {
        int* object2 = mPreObjMarkSolid.Data(last, j);
        for (int k = last + kFillFromBorder; k <= i - kFillFromBorder; k++) {
          *object2++ = mCurrentPreIndex;
        }
        last = i;
      }

      object++;
    }
  }

  for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
    const int* object = mPreObjMark.Data(i, mCurrentPre->Dimentions.Top);
    int last = mCurrentPre->Dimentions.Bottom;
    for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
      int obj = *object;
      if (obj == mCurrentPreIndex) {
        int* object2 = mPreObjMarkSolid.Data(i, last);
        for (int k = last + kFillFromBorder; k <= j - kFillFromBorder; k++) {
          *object2 = mCurrentPreIndex;
          object2 += mPreObjMarkSolid.Stride();
        }
        last = j;
      }

      object += mPreObjMark.Stride();
    }
  }
}

void LineObj::BorderPreObj()
{
  mPreObjMarkSolid.ZeroSource();
  for (int i = 0; i < mPreObj1.size(); i++) {
    mCurrentPreIndex = mPreObj1[i];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    BorderPreObjOne();
  }
}

void LineObj::BorderPreObjOne()
{
  for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
    const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left, j);
    int last = -1;
    for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
      int obj = *object;
      if (obj == mCurrentPreIndex) {
        if (last < 0) {
          *mPreObjMarkSolid.Data(i, j) = mCurrentPreIndex;
        }
        last = i;
      }

      object++;
    }
    if (last > 0) {
      *mPreObjMarkSolid.Data(last, j) = mCurrentPreIndex;
    }
  }

  for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
    const int* object = mPreObjMark.Data(i, mCurrentPre->Dimentions.Top);
    int last = -1;
    for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
      int obj = *object;
      if (obj == mCurrentPreIndex) {
        if (last < 0) {
          *mPreObjMarkSolid.Data(i, j) = mCurrentPreIndex;
        }
        last = j;
      }

      object += mPreObjMark.Stride();
    }
    if (last > 0) {
      *mPreObjMarkSolid.Data(i, last) = mCurrentPreIndex;
    }
  }
}

void LineObj::MakePreType1()
{
  for (int i = 0; i < mPreObj1.size(); i++) {
    mCurrentPreIndex = mPreObj1[i];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    MakePreType1One();
  }
}

void LineObj::MakePreType1One()
{
  int minMass = qMax(1, mCurrentPre->Count / 16);

  MakePreType1OneTop(mCurrentPre->Info.Top, minMass);
  MakePreType1OneBottom(mCurrentPre->Info.Bottom, minMass);
  MakePreType1OneLeft(mCurrentPre->Info.Left, minMass);
  MakePreType1OneRight(mCurrentPre->Info.Right, minMass);
//  mCurrentPre->Info.Center = (mCurrentPre->Info.Top + mCurrentPre->Info.Bottom + mCurrentPre->Info.Left + mCurrentPre->Info.Right) / 4;
}

void LineObj::MakePreType1OneTop(InfoPoint& cmass, int minMass)
{
  int count = 0;
  cmass.setX(0);
  cmass.setY(0);

  for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
    AddPreType1OneHorzLine(j, cmass, count);
    if (count >= minMass) {
      break;
    }
  }

  if (count) {
    cmass.rx() = cmass.x() / count;
    cmass.ry() = cmass.y() / count;
  }
}

void LineObj::MakePreType1OneBottom(InfoPoint& cmass, int minMass)
{
  int count = 0;
  cmass.setX(0);
  cmass.setY(0);

  for (int j = mCurrentPre->Dimentions.Bottom; j >= mCurrentPre->Dimentions.Top; j--) {
    AddPreType1OneHorzLine(j, cmass, count);
    if (count >= minMass) {
      break;
    }
  }

  if (count) {
    cmass.rx() = cmass.x() / count;
    cmass.ry() = cmass.y() / count;
  }
}

void LineObj::MakePreType1OneLeft(InfoPoint& cmass, int minMass)
{
  int count = 0;
  cmass.setX(0);
  cmass.setY(0);

  for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
    AddPreType1OneVertLine(i, cmass, count);
    if (count >= minMass) {
      break;
    }
  }

  if (count) {
    cmass.rx() = cmass.x() / count;
    cmass.ry() = cmass.y() / count;
  }
}

void LineObj::MakePreType1OneRight(InfoPoint& cmass, int minMass)
{
  int count = 0;
  cmass.setX(0);
  cmass.setY(0);

  for (int i = mCurrentPre->Dimentions.Right; i >= mCurrentPre->Dimentions.Left; i--) {
    AddPreType1OneVertLine(i, cmass, count);
    if (count >= minMass) {
      break;
    }
  }

  if (count) {
    cmass.rx() = cmass.x() / count;
    cmass.ry() = cmass.y() / count;
  }
}

void LineObj::AddPreType1OneHorzLine(int j, InfoPoint& cmass, int& count)
{
  const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left, j);
  for (int i = mCurrentPre->Dimentions.Left; i <= mCurrentPre->Dimentions.Right; i++) {
    if (*object == mCurrentPreIndex) {
      cmass.rx() += i;
      cmass.ry() += j;
      count++;
    }
    object++;
  }
}

void LineObj::AddPreType1OneVertLine(int i, InfoPoint& cmass, int& count)
{
  const int* object = mPreObjMark.Data(i, mCurrentPre->Dimentions.Top);
  int stride = mPreObjMark.Stride();
  for (int j = mCurrentPre->Dimentions.Top; j <= mCurrentPre->Dimentions.Bottom; j++) {
    if (*object == mCurrentPreIndex) {
      cmass.rx() += i;
      cmass.ry() += j;
      count++;
    }
    object += stride;
  }
}

class PreLength0More {
  const QVector<LineObj::PreObj>& mPreObj;

public:
  bool operator()(int i, int j) const
  {
    return mPreObj.at(i).Length0 > mPreObj.at(j).Length0;
  }

  PreLength0More(const QVector<LineObj::PreObj>& _PreObj)
    : mPreObj(_PreObj)
  { }
};

void LineObj::ConnectLines()
{
  mObj.clear();

  PreLength0More comp(mPreObj);
  std::sort(mPreObj1.begin(), mPreObj1.end(), comp);

  foreach (mCurrentPreIndex, mPreObj1) {
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (!mCurrentPre->Taken) {
      ConnectLineOne();
    }
  }
}

void LineObj::ConnectLineOne()
{
  //mLineTmp.SetSize(mCurrentPre->Dimentions.Width(), mCurrentPre->Dimentions.Height());
  //mTmpBasePoint.setX(mCurrentPre->Dimentions.Left);
  //mTmpBasePoint.setY(mCurrentPre->Dimentions.Top);

  //QPoint midPoint;
  //FindMidPoint(midPoint);
  //QPoint startPoint;
  //MarkPoints(midPoint, startPoint);
  //QPoint finishPoint;
  //MarkPoints(startPoint, finishPoint);
  //mObj.append(Obj());
  //ApplyPath(finishPoint, &mObj.last().Points);
  //MarkPath(&mObj.last().Points);

  //if (mCurrentPreLength >= mMinObjLength) {
  //  QLinkedList<Obj*> usedObjs;
  //  usedObjs.append(&mObj.last());

  //  QPoint endPoint;
  //  while (FindPathEndPoint(endPoint)) {
  //    mObj.append(Obj());
  //    QLinkedList<QPoint>* brunchPoints = &mObj.last().Points;
  //    ApplyPath(endPoint, brunchPoints);
  //    SplitPath(usedObjs, brunchPoints->last());
  //    MarkPath(brunchPoints);
  //  }
  //}
}

void LineObj::FindMidPoint(QPoint& midPoint)
{
  int jPoint1 = (mCurrentPre->Dimentions.Top + mCurrentPre->Dimentions.Bottom) / 2;
  int iPoint1 = (mCurrentPre->Dimentions.Left + mCurrentPre->Dimentions.Right) / 2;
  int w2 = mCurrentPre->Dimentions.Width()/2;

  const int* object = mPreObjMark.Line(jPoint1);
  if (object[iPoint1] != mCurrentPreIndex) {
    for (int i = 1; i <= w2; i++) {
      if (object[iPoint1 + i] == mCurrentPreIndex) {
        iPoint1 += i;
        break;
      } else if (object[iPoint1 - i] == mCurrentPreIndex) {
        iPoint1 -= i;
        break;
      }
    }
  }
  midPoint.setX(iPoint1 - mCurrentPre->Dimentions.Left);
  midPoint.setY(jPoint1 - mCurrentPre->Dimentions.Top);
}

void LineObj::MarkPoints(const QPoint& startPoint, QPoint& endPoint)
{
  QLinkedList<QPoint> currentPath;
  currentPath.append(startPoint);

  MarkObjPointsLength(currentPath);
  endPoint = currentPath.first();
}

void LineObj::MarkPointsLength(QLinkedList<QPoint>& currentPoints)
{
  int width = mLineTmp.Width();
  int height = mLineTmp.Height();
  int widthLim = width - 1;
  int heightLim = height - 1;

  mLineTmp.SetData(0x7f7f7f7f);
  foreach (const QPoint& p, currentPoints) {
    *mLineTmp.Data(p) = 0;
  }

  forever {
    QLinkedList<QPoint> nextPath;
    foreach (const QPoint& p, currentPoints) {
      int* data = mLineTmp.Data(p);
      int length = *data;

      if (p.x() > 0 && data[-1] > length + 2) {
        data[-1] = length + 2;
        nextPath.append(QPoint(p.x()-1, p.y()));
      }
      if (p.x() < widthLim && data[1] > length + 2) {
        data[1] = length + 2;
        nextPath.append(QPoint(p.x()+1, p.y()));
      }
      if (p.y() > 0 && data[-width] > length + 2) {
        data[-width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()-1));
      }
      if (p.y() < heightLim && data[width] > length + 2) {
        data[width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()+1));
      }
    }
    if (nextPath.isEmpty()) {
      return;
    }
    currentPoints = nextPath;
  }
}

void LineObj::MarkObjPointsLength(QLinkedList<QPoint>& currentPoints)
{
  int stride = mPreObjMark.Stride();
  int width = mLineTmp.Width();
  int height = mLineTmp.Height();
  int widthLim = width - 1;
  int heightLim = height - 1;

  mLineTmp.SetData(0x7f7f7f7f);
  foreach (const QPoint& p, currentPoints) {
    *mLineTmp.Data(p) = 0;
  }

  mCurrentPreLength = 0;
  forever {
    QLinkedList<QPoint> nextPath;
    foreach (const QPoint& p, currentPoints) {
      const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left + p.x(), mCurrentPre->Dimentions.Top + p.y());
      int* data = mLineTmp.Data(p);
      int length = *data;

      if (p.x() > 0 && object[-1] == mCurrentPreIndex && data[-1] > length + 2) {
        data[-1] = length + 2;
        nextPath.append(QPoint(p.x()-1, p.y()));
      }
      if (p.x() < widthLim && object[1] == mCurrentPreIndex && data[1] > length + 2) {
        data[1] = length + 2;
        nextPath.append(QPoint(p.x()+1, p.y()));
      }
      if (p.y() > 0 && object[-stride] == mCurrentPreIndex && data[-width] > length + 2) {
        data[-width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()-1));
      }
      if (p.y() < heightLim && object[stride] == mCurrentPreIndex && data[width] > length + 2) {
        data[width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()+1));
      }
    }
    if (nextPath.isEmpty()) {
      return;
    }
    currentPoints = nextPath;
    mCurrentPreLength++;
  }
}

void LineObj::MarkPath(QLinkedList<QPoint>* points)
{
  int stride = mPreObjMark.Stride();
  int width = mLineTmp.Width();
  int height = mLineTmp.Height();
  int widthLim = width - 1;
  int heightLim = height - 1;

  QLinkedList<QPoint> currentPath;
  foreach (const QPoint& p, *points) {
    QPoint tmpP = p - mTmpBasePoint;
    *mLineTmp.Data(tmpP) = 0;
    currentPath.append(tmpP);
  }
  mCurrentPreLength = 0;
  forever {
    QLinkedList<QPoint> nextPath;
    foreach (const QPoint& p, currentPath) {
      const int* object = mPreObjMark.Data(mCurrentPre->Dimentions.Left + p.x(), mCurrentPre->Dimentions.Top + p.y());
      int* data = mLineTmp.Data(p);
      int length = *data;

      if (p.x() > 0 && object[-1] == mCurrentPreIndex && data[-1] > length + 2) {
        data[-1] = length + 2;
        nextPath.append(QPoint(p.x()-1, p.y()));
      }
      if (p.x() < widthLim && object[1] == mCurrentPreIndex && data[1] > length + 2) {
        data[1] = length + 2;
        nextPath.append(QPoint(p.x()+1, p.y()));
      }
      if (p.y() > 0 && object[-stride] == mCurrentPreIndex && data[-width] > length + 2) {
        data[-width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()-1));
      }
      if (p.y() < heightLim && object[stride] == mCurrentPreIndex && data[width] > length + 2) {
        data[width] = length + 2;
        nextPath.append(QPoint(p.x(), p.y()+1));
      }
    }
    if (nextPath.isEmpty()) {
      break;
    }
    currentPath = nextPath;
    mCurrentPreLength++;
  }
}

void LineObj::DumpMark()
{
  static int gIndex = 0;
  QFile tmp(QString("V:/Temp/va/%1.txt").arg(++gIndex, 4, 10, QChar('0')));
  tmp.open(QFile::WriteOnly);
  for (int j = 0; j < mLineTmp.Height(); j++) {
    QString line;
    const int* src = mLineTmp.Line(j);
    for (int i = 0; i < mLineTmp.Width(); i++) {
      line.append(QString("%1, ").arg(*src, 2));

      src++;
    }
    line.append('\n');
    tmp.write(line.toLatin1());
  }
}

bool LineObj::FindPathEndPoint(QPoint& endPoint)
{
  int maxLength = 0;
  for (int j = 0; j < mLineTmp.Height(); j++) {
    const int* src = mLineTmp.Line(j);
    for (int i = 0; i < mLineTmp.Width(); i++) {
      if (*src != 0x7f7f7f7f && *src > maxLength) {
        endPoint.setX(i);
        endPoint.setY(j);
        maxLength = *src;
      }
      src++;
    }
  }
  return maxLength > mMinObjLength;
}

void LineObj::ApplyPath(const QPoint& endPoint, QLinkedList<QPoint>* points)
{
  int width = mLineTmp.Width();
  int height = mLineTmp.Height();
  int widthLim = width - 1;
  int heightLim = height - 1;

  QPoint p = endPoint;
  int* data = mLineTmp.Data(p);
  int length = *data;
  forever {
    points->append(mTmpBasePoint + p);

    if (p.x() > 0 && data[-1] <= length - 2) {
      p.rx()--;
      data--;
      length = *data;
    } else if (p.x() < widthLim && data[1] <= length - 2) {
      p.rx()++;
      data++;
      length = *data;
    } else if (p.y() > 0 && data[-width] <= length - 2) {
      p.ry()--;
      data -= width;
      length = *data;
    } else if (p.y() < heightLim && data[width] <= length - 2) {
      p.ry()++;
      data += width;
      length = *data;
    } else {
      break;
    }
  }
}

//void LineObj::SplitPath(QLinkedList<LineObj::Obj*>& usedObjs, const QPoint& splitPoint)
//{
//  for (auto itr = usedObjs.begin(); itr != usedObjs.end(); itr++) {
//    LineObj::Obj* obj = *itr;
//    int prePointsCount = 0;
//    for (auto itr = obj->Points.begin(); itr != obj->Points.end(); itr++) {
//      const QPoint& p = *itr;
//      if (p == splitPoint) {
//        if (prePointsCount < mMinObjLength) {
//          obj->Points.erase(obj->Points.begin(), itr);
//        } else if (obj->Points.size() - prePointsCount < mMinObjLength) {
//          obj->Points.erase(itr, obj->Points.end());
//        } else {
//          mObj.append(Obj());
//          Obj* newObj = &mObj.last();
//          usedObjs.append(newObj);
//          while (itr != obj->Points.end()) {
//            newObj->Points.append(*itr);
//            itr = obj->Points.erase(itr);
//          }
//        }
//        return;
//      }
//      prePointsCount++;
//    }
//  }
//}

void LineObj::ObjPrepareOne()
{
  //mCurrentObj->NewInfo = mCurrentObj->Info;
}

void LineObj::ObjTakePre()
{
  for (mCurrentObjIndex = 0; mCurrentObjIndex < mObj.size(); mCurrentObjIndex++) {
    mCurrentObj = &mObj[mCurrentObjIndex];
    ObjPrepareOne();
    ObjTakePreOne();
  }
}

void LineObj::ObjTakePreOne()
{
  int dx = (mCurrentObj->Info.Right.x() - mCurrentObj->Info.Left.x() + 1) / 2;
  int dy = (mCurrentObj->Info.Bottom.y() - mCurrentObj->Info.Top.y() + 1) / 2;
  Q_ASSERT(kPoinsCount == 4);

  for (int pre1Index = 0; pre1Index < mPreObj1.size(); pre1Index++) {
    mCurrentPreIndex = mPreObj1[pre1Index];
    mCurrentPre = &mPreObj[mCurrentPreIndex];

    if (mCurrentPre->Info.Left.x() > mCurrentObj->Info.Left.x() - dx
        && mCurrentPre->Info.Left.x() < mCurrentObj->Info.Right.x() + dx
        && mCurrentPre->Info.Left.y() > mCurrentObj->Info.Top.y() - dy
        && mCurrentPre->Info.Left.y() < mCurrentObj->Info.Bottom.y() + dy) {
      ObjAddRefPrePoint(eLeftPoint);
    }
    if (mCurrentPre->Info.Right.x() < mCurrentObj->Info.Right.x() + dx
        && mCurrentPre->Info.Right.x() > mCurrentObj->Info.Left.x() - dx
        && mCurrentPre->Info.Right.y() > mCurrentObj->Info.Top.y() - dy
        && mCurrentPre->Info.Right.y() < mCurrentObj->Info.Bottom.y() + dy) {
      ObjAddRefPrePoint(eRightPoint);
    }
    if (mCurrentPre->Info.Top.y() > mCurrentObj->Info.Top.y() - dy
        && mCurrentPre->Info.Top.y() < mCurrentObj->Info.Bottom.y() + dy
        && mCurrentPre->Info.Top.x() > mCurrentObj->Info.Left.x() - dx
        && mCurrentPre->Info.Top.x() < mCurrentObj->Info.Right.x() + dx) {
      ObjAddRefPrePoint(eTopPoint);
    }
    if (mCurrentPre->Info.Bottom.y() < mCurrentObj->Info.Bottom.y() + dy
        && mCurrentPre->Info.Bottom.y() > mCurrentObj->Info.Top.y() - dy
        && mCurrentPre->Info.Bottom.x() > mCurrentObj->Info.Left.x() - dx
        && mCurrentPre->Info.Bottom.x() < mCurrentObj->Info.Right.x() + dx) {
      ObjAddRefPrePoint(eBottomPoint);
    }
  }
}

void LineObj::ObjAddRefPrePoint(int pointPosition)
{
  mCurrentPre->Taken = true;

  {
    auto itr = mPreObjPointRef.find(mCurrentPreIndex);
    if (itr == mPreObjPointRef.end()) {
      itr = mPreObjPointRef.insert(mCurrentPreIndex, ObjRef());
    }
    itr->PointsRef[pointPosition].append(mCurrentObjIndex);
  }

  {
    auto itr = mObjPrePointRef.find(mCurrentObjIndex);
    if (itr == mObjPrePointRef.end()) {
      itr = mObjPrePointRef.insert(mCurrentObjIndex, ObjRef());
    }
    itr->PointsRef[pointPosition].append(mCurrentPreIndex);
  }
}

void LineObj::ObjApplyPre()
{
  for (auto itr = mPreObjPointRef.begin(); itr != mPreObjPointRef.end(); itr++) {
    mCurrentPreIndex = itr.key();
    ObjPreRefValidateOne(&itr.value());
  }

  for (mCurrentObjIndex = 0; mCurrentObjIndex < mObj.size(); mCurrentObjIndex++) {
    mCurrentObj = &mObj[mCurrentObjIndex];
    ObjApplyPreOne();
  }
}

void LineObj::ObjPreRefValidateOne(LineObj::ObjRef* objRef)
{
  int maxMass = 0;
  for (int i = 0; i < kPoinsCount; i++) {
    foreach (mCurrentObjIndex, objRef->PointsRef[i]) {
      mCurrentObj = &mObj[mCurrentObjIndex];
      if (mCurrentObj->Mass > maxMass) {
        maxMass = mCurrentObj->Mass;
      }
    }
  }

  for (int i = 0; i < kPoinsCount; i++) {
    for (auto itr = objRef->PointsRef[i].begin(); itr != objRef->PointsRef[i].end(); ) {
      mCurrentObj = &mObj[mCurrentObjIndex];
      if (mCurrentObj->Mass < maxMass / 4) {
        mObjPrePointRef[mCurrentObjIndex].PointsRef[i].removeOne(mCurrentPreIndex);
        itr = objRef->PointsRef[i].erase(itr);
      } else {
        itr++;
      }
    }
  }

//  QSet<int> usedIds[kPoinsCount/2];
//  for (int i = 0; i < kPoinsCount/2; i++) {
//    for (auto itr = objRef->PointsRef[i].begin(); itr != objRef->PointsRef[i].end(); ) {
//      mCurrentObjIndex = *itr;
//      mCurrentObj = &mObj[mCurrentObjIndex];
//      if (mCurrentObj->Mass > maxMass / 4) {
//        usedIds[i].insert(mCurrentObjIndex);
//        itr++;
//      } else {
//        itr = objRef->PointsRef[i].erase(itr);
//      }
//    }
//  }
//  for (int i = kPoinsCount/2; i < kPoinsCount; i++) {
//    for (auto itr = objRef->PointsRef[i].begin(); itr != objRef->PointsRef[i].end(); ) {
//      mCurrentObjIndex = *itr;
//      mCurrentObj = &mObj[mCurrentObjIndex];
//      if (mCurrentObj->Mass > maxMass / 4) {
//        int iv = i - kPoinsCount/2;
//        if (usedIds[iv].size() > 1 || (usedIds[iv].size() == 1 && !usedIds[iv].contains(mCurrentObjIndex))) {
//          itr = objRef->PointsRef[i].erase(itr);
//        }
//        usedIds[i].insert(mCurrentObjIndex);
//        itr++;
//      } else {
//        itr = objRef->PointsRef[i].erase(itr);
//      }
//    }
//  }
}

void LineObj::ObjApplyPreOne()
{
  int dx = (mCurrentObj->Info.Right.x() - mCurrentObj->Info.Left.x() + 1) / 2;
  int dy = (mCurrentObj->Info.Bottom.y() - mCurrentObj->Info.Top.y() + 1) / 2;
//  Q_ASSERT(kPoinsCount == 4);
//  const int rr[kPoinsCount] = { dy, dx, dy, dx };
//  const bool hz[kPoinsCount] = { false, true, false, true };

  auto itrObjRef = mObjPrePointRef.find(mCurrentObjIndex);
  if (itrObjRef == mObjPrePointRef.end()) {
    return;
  }

  ObjRef* objRef = &*itrObjRef;

  int minTop = mCurrentObj->Info.Bottom.y();
  foreach (mCurrentPreIndex, objRef->PointsRef[eTopPoint]) {
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (qAbs(mCurrentObj->Info.Top.y() - mCurrentPre->Info.Top.y()) < dy) {
      minTop = qMin(minTop, mCurrentPre->Info.Top.y());
    }
  }
  if (minTop < mCurrentObj->Info.Bottom.y()) {
    mCurrentObj->Info.Top.ry() = minTop;
  }
  int minBottom = mCurrentObj->Info.Top.y();
  foreach (mCurrentPreIndex, objRef->PointsRef[eBottomPoint]) {
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (qAbs(mCurrentObj->Info.Bottom.y() - mCurrentPre->Info.Bottom.y()) < dy) {
      minBottom = qMax(minBottom, mCurrentPre->Info.Bottom.y());
    }
  }
  if (minBottom > mCurrentObj->Info.Top.y()) {
    mCurrentObj->Info.Bottom.ry() = minBottom;
  }

  int minLeft = mCurrentObj->Info.Right.x();
  foreach (mCurrentPreIndex, objRef->PointsRef[eLeftPoint]) {
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (qAbs(mCurrentObj->Info.Left.x() - mCurrentPre->Info.Left.x()) < dx) {
      minLeft = qMin(minLeft, mCurrentPre->Info.Left.x());
    }
  }
  if (minLeft < mCurrentObj->Info.Right.x()) {
    mCurrentObj->Info.Left.rx() = minLeft;
  }
  int minRight = mCurrentObj->Info.Left.x();
  foreach (mCurrentPreIndex, objRef->PointsRef[eRightPoint]) {
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (qAbs(mCurrentObj->Info.Right.x() - mCurrentPre->Info.Right.x()) < dx) {
      minRight = qMax(minRight, mCurrentPre->Info.Right.x());
    }
  }
  if (minRight > mCurrentObj->Info.Left.x()) {
    mCurrentObj->Info.Right.rx() = minRight;
  }
}

void LineObj::ObjFromUnusedPre()
{
  for (int pre1Index = 0; pre1Index < mPreObj1.size(); pre1Index++) {
    mCurrentPreIndex = mPreObj1[pre1Index];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (!mCurrentPre->Taken) {
      ObjFromUnusedPreOne();
    }
  }
}

void LineObj::ObjFromUnusedPreOne()
{
  mObj.append(Obj());
  mCurrentObjIndex = mObj.size() - 1;
  mCurrentObj = &mObj[mCurrentObjIndex];
  mCurrentObj->Type = eCasper;
  mCurrentObj->Info = mCurrentPre->Info;
  mCurrentObj->Mass = mCurrentPre->Count;
  mCurrentPre->Taken = true;
  ObjSetTypeIfNearBorder(eTest);

  QLinkedList<PreObj*> takenPre;
  ObjFromUnusedPreTakePre(mCurrentObj->Info, takenPre);
  foreach (mCurrentPre, takenPre) {
    if (mCurrentObj->Type == eCasper) {
      ObjSetTypeIfNearBorder(eTest);
    }
    mCurrentObj->Mass += mCurrentPre->Count;
    if (mCurrentPre->Info.Left.x() < mCurrentObj->Info.Left.x()) {
      mCurrentObj->Info.Left = mCurrentPre->Info.Left;
    }
    if (mCurrentPre->Info.Right.x() > mCurrentObj->Info.Right.x()) {
      mCurrentObj->Info.Right = mCurrentPre->Info.Right;
    }
    if (mCurrentPre->Info.Top.y() < mCurrentObj->Info.Top.y()) {
      mCurrentObj->Info.Top = mCurrentPre->Info.Top;
    }
    if (mCurrentPre->Info.Bottom.y() > mCurrentObj->Info.Bottom.y()) {
      mCurrentObj->Info.Bottom = mCurrentPre->Info.Bottom;
    }
  }
}

void LineObj::ObjFromUnusedPreTakePre(const Type1Info& obj1, QLinkedList<PreObj*>& takenPre)
{
  for (int pre1Index = 0; pre1Index < mPreObj1.size(); pre1Index++) {
    mCurrentPreIndex = mPreObj1[pre1Index];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    if (!mCurrentPre->Taken) {
      ObjFromUnusedPreTakePreOne(obj1, takenPre);
    }
  }
}

void LineObj::ObjFromUnusedPreTakePreOne(const Type1Info& obj1, QLinkedList<LineObj::PreObj*>& takenPre)
{
  int dxo = (obj1.Right.x() - obj1.Left.x() + 1) / 2;
  int dyo = (obj1.Bottom.y() - obj1.Top.y() + 1) / 2;
  int ro = qMax(dxo, dyo);
  int dxp = (mCurrentPre->Info.Right.x() - mCurrentPre->Info.Left.x() + 1) / 2;
  int dyp = (mCurrentPre->Info.Bottom.y() - mCurrentPre->Info.Top.y() + 1) / 2;
  int rp = qMax(dxp, dyp);
  int r = 3 * qMax(ro, rp);

  for (int i = 0; i < kPoinsCount; i++) {
    const InfoPoint& po = obj1.Points[i];
    for (int j = 0; j < kPoinsCount; j++) {
      const InfoPoint& pp = mCurrentPre->Info.Points[j];
      if (po.manhattanLengthTo(pp) <= r) {
        takenPre.append(mCurrentPre);
        mCurrentPre->Taken = true;
        ObjFromUnusedPreTakePre(mCurrentPre->Info, takenPre);
        return;
      }
    }
  }
}

bool LineObj::ObjSetTypeIfNearBorder(LineObj::ObjType newType)
{
  if ((GetBlockInfo().BlockData(mCurrentPre->Info.Left.x(), mCurrentPre->Info.Left.y())->Flag & BlockInfo::eDoor)
      || (GetBlockInfo().BlockData(mCurrentPre->Info.Right.x(), mCurrentPre->Info.Right.y())->Flag & BlockInfo::eDoor)
      || (GetBlockInfo().BlockData(mCurrentPre->Info.Top.x(), mCurrentPre->Info.Top.y())->Flag & BlockInfo::eDoor)
      || (GetBlockInfo().BlockData(mCurrentPre->Info.Bottom.x(), mCurrentPre->Info.Bottom.y())->Flag & BlockInfo::eDoor)) {
    mCurrentObj->Type = newType;
    return true;
  }
  return false;
}

void LineObj::GetDbgPreObjMark(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar*     dbg    = debug.Line(j);
    const int* object = mPreObjMark.Line(j);

    for (int i = 0; i < Width(); i++) {
      *dbg = 0;
      if (*object) {
        int obj = *object;
        if (obj == 666666) {
          *dbg = 0xff;
        } else if (mPreObj1Map.at(obj) >= 0) {
          *dbg = (((obj % 4) + 1) * 32 - 1);
        } else {
          *dbg = (((obj % 4) + 1) * 32 - 1) | 0x80;
        }
      }
      dbg++;
      object++;
    }
  }
}

void LineObj::GetDbgPreObjMarkSolid(ImageSrc<uchar>& debug)
{
  for (int j = 0; j < Height(); j++) {
    uchar*     dbg    = debug.Line(j);
    const int* object = mPreObjMarkSolid.Line(j);

    for (int i = 0; i < Width(); i++) {
      *dbg = 0;
      if (*object) {
        int obj = *object;
        if (mPreObj1Map.at(obj) >= 0) {
          *dbg = (((obj % 4) + 1) * 32 - 1);
        } else {
          *dbg = (((obj % 4) + 1) * 32 - 1) | 0x80;
        }
      }
      dbg++;
      object++;
    }
  }
}

void LineObj::GetDbgPreObj(ImageSrc<uchar>& debug)
{
  debug.ZeroSource();
  if (mPreObj.isEmpty()) {
    return;
  }
  foreach (const PreObj& pre, mPreObj) {
    {
      int j = pre.Dimentions.Top;
      uchar* dbg = debug.Data(pre.Dimentions.Left, j);
      for (int i = 0; i < pre.Dimentions.Width(); i++) {
        *dbg++ = 3;
      }
    }
    {
      int j = pre.Dimentions.Bottom;
      uchar* dbg = debug.Data(pre.Dimentions.Left, j);
      for (int i = 0; i < pre.Dimentions.Width(); i++) {
        *dbg++ = 3;
      }
    }
    {
      int i = pre.Dimentions.Left;
      uchar* dbg = debug.Data(i, pre.Dimentions.Top);
      for (int j = 0; j < pre.Dimentions.Height(); j++) {
        *dbg = 3;
        dbg += debug.Stride();
      }
    }
    {
      int i = pre.Dimentions.Right;
      uchar* dbg = debug.Data(i, pre.Dimentions.Top);
      for (int j = 0; j < pre.Dimentions.Height(); j++) {
        *dbg = 3;
        dbg += debug.Stride();
      }
    }
  }
}

void LineObj::GetDbgAllObjType1(ImageSrc<uchar>& debug)
{
  debug.ZeroSource();
  for (int i = 0; i < mPreObj1.size(); i++) {
    mCurrentPreIndex = mPreObj1[i];
    mCurrentPre = &mPreObj[mCurrentPreIndex];
    uchar color = (((i % 4) + 1) * 32 - 1);
    GetDbgType1One(debug, mCurrentPre->Info, color);
  }

  for (mCurrentObjIndex = 0; mCurrentObjIndex < mObj.size(); mCurrentObjIndex++) {
    mCurrentObj = &mObj[mCurrentObjIndex];
    uchar color = (mCurrentObj->Type == eCasper)? 0xb0: 0xf0;
    GetDbgType1One(debug, mCurrentObj->Info, color, 1);
  }
}

void LineObj::GetDbgType1One(ImageSrc<uchar>& debug, const Type1Info& info, uchar color, int inc)
{
  debug.FillLine(color, info.Left.x() - inc, info.Left.y(), info.Top.x(), info.Top.y() - inc);
  debug.FillLine(color, info.Top.x(), info.Top.y() - inc, info.Right.x() + inc, info.Right.y());
  debug.FillLine(color, info.Right.x() + inc, info.Right.y(), info.Bottom.x(), info.Bottom.y() + inc);
  debug.FillLine(color, info.Bottom.x(), info.Bottom.y() + inc, info.Left.x() - inc, info.Left.y());
}

void LineObj::GetDbgPreHyst(byte* data)
{
  int* datai = reinterpret_cast<int*>(data);
  datai[0] = mPreMomentHyst.TotalCount() / 4;
  memset(datai + 1, 0, sizeof(int) * 256);
  memcpy(datai + 1, mPreMomentHyst.Data(), sizeof(int) * mPreMomentHyst.Size());

  int level1 = mPreMomentHyst.GetValue(500);
  datai[1 + level1] = mPreMomentHyst.TotalCount() / 4;
  datai[1 + level1+1] = mPreMomentHyst.TotalCount() / 4;

  int level2 = mPreMomentHyst.GetValue(900);
  datai[1 + level2] = mPreMomentHyst.TotalCount() / 4;
  datai[1 + level2+1] = mPreMomentHyst.TotalCount() / 4;
}

void LineObj::GetDbgPreHyst2(byte* data)
{
  int* datai = reinterpret_cast<int*>(data);
  datai[0] = mPreMomentHyst2.TotalCount() / 4;
  memset(datai + 1, 0, sizeof(int) * 256);
  memcpy(datai + 1, mPreMomentHyst2.Data(), sizeof(int) * mPreMomentHyst2.Size());
}

void LineObj::GetDbgObj(ImageSrc<uchar>& debug)
{
  debug.ZeroSource();
  if (mObj.isEmpty()) {
    return;
  }
  //foreach (const Obj& obj, mObj) {
  //  foreach (const InfoPoint& p, obj.Points) {
  //    *debug.Data(p.x(), p.y()) = 2;
  //  }
  //}
}

LineObj::LineObj(const AnalyticsB& _Analytics, const ImageSrc<uchar>& _LayerF)
  : BlockSceneAnalizer(_Analytics)
  , mLayerF(_LayerF), mLayerMark(GetBlockScene()), mPreObjMark(GetBlockScene()), mPreObjMarkSolid(GetBlockScene())
  , mMarkConnect(true)
{
}

