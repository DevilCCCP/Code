#include "ObjConnect.h"


void ObjConnect::ConnectBlack(const Region<uchar>& region, int maxValue)
{
  SetBlack(region, maxValue);
  Connect();
  Construct();
}

bool ObjConnect::FillObjectBlack(int index, const QRect& rect, Region<uchar>* region)
{
  if (index >= mObjIds.size()) {
    return false;
  }

  const QRect& objRect = mObjList.at(index);
  if (rect.width() < objRect.width() || rect.height() < objRect.height()) {
    return false;
  }
  for (int j = 0; j < objRect.height(); j++) {
    const int* object = mRegionMark.Data(objRect.left(), objRect.top() + j);
    uchar* dst = region->Data(rect.left(), rect.top() + j);

    for (int i = 0; i < objRect.width(); i++) {
      if (*object && mObjIds[*object] == index) {
        *dst = 0;
      }

      object++;
      dst++;
    }
  }
  return true;
}

void ObjConnect::SetBlack(const Region<uchar>& region, int maxValue)
{
  mRegionMark.SetSize(region.Width(), region.Height());
  mRegionMark.ZeroData();

  mObjCount = 0;
  for (int j = 0; j < region.Height(); j++) {
    const uchar* src = region.Line(j);
    int* object = mRegionMark.Line(j);

    bool inObject = false;
    for (int i = 0; i < region.Width(); i++) {
      bool isObject = (*src <= maxValue);
      if (isObject) {
        if (!inObject) {
          ++mObjCount;
          inObject = true;
        }
        *object = mObjCount;
      } else {
        if (inObject) {
          inObject = false;
        }
      }

      src++;
      object++;
    }
  }
}

void ObjConnect::Connect()
{
  mObjIds.resize(mObjCount + 1);
  mObjIds.fill(0);

  for (int j = 0; j < mRegionMark.Height() - 1; j++) {
    int* object1 = mRegionMark.Line(j);
    int* object2 = mRegionMark.Line(j + 1);

    for (int i = 0; i < mRegionMark.Width(); i++) {
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

  int realObjCount = 0;
  for (int obj = 1; obj <= mObjCount; obj++) {
    if (int objRef = mObjIds[obj]) {
      mObjIds[obj] = mObjIds[objRef];
    } else {
      mObjIds[obj] = realObjCount;
      realObjCount++;
    }
  }
  mObjCount = realObjCount;
}

void ObjConnect::Construct()
{
  mObjList.resize(mObjCount);
  mObjList.fill(QRect());

  for (int j = 0; j < mRegionMark.Height(); j++) {
    const int* object = mRegionMark.Line(j);

    for (int i = 0; i < mRegionMark.Width(); i++) {
      if (*object) {
        int iObj = mObjIds[*object];
        QRect* obj = &mObjList[iObj];
        if (!obj->isEmpty()) {
          obj->setLeft(qMin(obj->left(), i));
          obj->setRight(qMax(obj->right(), i));
          obj->setTop(qMin(obj->top(), j));
          obj->setBottom(qMax(obj->bottom(), j));
        } else {
          obj->setCoords(i, j, i, j);
        }
      }

      object++;
    }
  }
}

ObjConnect::ObjConnect(ImgAnalizer* _ImgAnalizer)
  : mImgAnalizer(_ImgAnalizer)
{
}

