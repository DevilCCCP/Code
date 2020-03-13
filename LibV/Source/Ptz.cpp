#include "Ptz.h"
#include "Ptz/PtzOnvif.h"


bool Ptz::GetAbilities(int& _AbilityFlag)
{
  _AbilityFlag = 0;

  return true;
}

bool Ptz::GetPosition(PtzVector& _Position)
{
  Q_UNUSED(_Position);

  return false;
}

bool Ptz::GetRange(PtzVector& _MinPosition, PtzVector& _MaxPosition, PtzVector& _MinSpeed, PtzVector& _MaxSpeed)
{
  Q_UNUSED(_MinPosition);
  Q_UNUSED(_MaxPosition);
  Q_UNUSED(_MinSpeed);
  Q_UNUSED(_MaxSpeed);

  return false;
}

bool Ptz::GetHome(PtzVector& _HomePosition)
{
  Q_UNUSED(_HomePosition);

  return false;
}

bool Ptz::SetPosition(const PtzVector& _Position)
{
  Q_UNUSED(_Position);

  return false;
}

bool Ptz::SetPosition(const PtzVector& _Position, const PtzVector& _Speed)
{
  Q_UNUSED(_Position);
  Q_UNUSED(_Speed);

  return false;
}

bool Ptz::RelativeMove(const PtzVector& _Position)
{
  Q_UNUSED(_Position);

  return false;
}

bool Ptz::RelativeMove(const PtzVector& _Position, const PtzVector& _Speed)
{
  Q_UNUSED(_Position);
  Q_UNUSED(_Speed);

  return false;
}

bool Ptz::ContinuousMove(const PtzVector& _Speed, int _Timeout)
{
  Q_UNUSED(_Speed);
  Q_UNUSED(_Timeout);

  return false;
}

bool Ptz::StopMove()
{
  return false;
}

bool Ptz::MoveHome()
{
  return false;
}

bool Ptz::MoveHome(const PtzVector& _Speed)
{
  Q_UNUSED(_Speed);

  return false;
}

bool Ptz::SetHome(const PtzVector& _Position)
{
  Q_UNUSED(_Position);

  return false;
}

PtzS Ptz::CreatePtz(SettingsA& settings)
{
  int type = settings.GetValue("Type", 0).toInt();
  switch (type) {
  case 0:  return PtzS(new PtzOnvif(settings));
  default: throw FatalException();
  }
}


Ptz::Ptz()
  : Imp(1000)
{
}

Ptz::~Ptz()
{
}

