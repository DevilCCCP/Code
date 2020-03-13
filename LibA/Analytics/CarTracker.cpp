#include "CarTracker.h"


void CarTracker::LoadSettings(const SettingsAS& settings)
{
  Q_UNUSED(settings);
}

void CarTracker::SaveSettings(const SettingsAS& settings)
{
  Q_UNUSED(settings);
}

void CarTracker::Init()
{

}

void CarTracker::Analize(const QList<CarInfo>& carList)
{

}

bool CarTracker::HaveObj()
{
//  for (; mReturnObjItr < mObjs.size(); mReturnObjItr++) {
//    mCurrentObj = &mObjs[mReturnObjItr];
//    if (qMin(mCurrentObj->Place.width(), mCurrentObj->Place.height()) >= mNormalMoment) {
//      return true;
//    }
//  }
//  mReturnObjItr = 0;
  return false;
}

bool CarTracker::RetrieveObj(Object& object)
{
  if (!HaveObj()) {
    return false;
  }

  object.Id = 0;
  object.Color = 0;
  object.Dimention = Rectangle(0, 0, 0, 0);
//  mCurrentObj = &mObjs[mReturnObjItr++];
//  object.Id = mCurrentObj->Id;
//  object.Color = ObjGetColor();
//  object.Dimention.Left   = mCurrentObj->Place.left();
//  object.Dimention.Top    = mCurrentObj->Place.top();
//  object.Dimention.Right  = mCurrentObj->Place.right();
//  object.Dimention.Bottom = mCurrentObj->Place.bottom();
  return true;
}


CarTracker::CarTracker(const AnalyticsB& _Analytics)
  : BlockSceneAnalizer(_Analytics)
{
}

