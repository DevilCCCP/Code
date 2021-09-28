#pragma once

#include <Lib/Settings/SettingsA.h>
#include <Lib/Dispatcher/Imp.h>


enum PtzCommand {
  eGetAbilities    = 0,
  eGetPosition     = 1 << 8,
  eGetRange        = 1 << 9,
  eGetHome         = 1 << 10,
  eSetPosition     = 1 << 16,
  eRelativeMove    = 1 << 17,
  eContinuousMove  = 1 << 18,
  eStopMove        = 1 << 19,
  eMoveHome        = 1 << 20,
  eSetHome         = 1 << 21,
  eUseSpeed        = 1 << 24
};

#pragma pack(push, 2)
struct PtzVector {
  float PosX;
  float PosY;
  float PosZ;

  static float NoValue() { return qQNaN(); }
  static bool IsValid(float value) { return !qIsNaN(value); }
  PtzVector(float _PosX, float _PosY, float _PosZ): PosX(_PosX), PosY(_PosY), PosZ(_PosZ) { }
  PtzVector(): PosX(qQNaN()), PosY(qQNaN()), PosZ(qQNaN()) { }
};

struct PtzRequest {
  int       Command;
  PtzVector Position;
  PtzVector Speed;
  int       Timeout;

  PtzRequest(): Command(0), Timeout(0) { }
};

struct PtzRespond {
  int       AbilityFlag;
  PtzVector Position;
  PtzVector MinPosition;
  PtzVector MaxPosition;
  PtzVector HomePosition;
  PtzVector MinSpeed;
  PtzVector MaxSpeed;
};
#pragma pack(pop)

DefineClassS(Ptz);
DefineClassS(Source);

class Ptz: public Imp
{
  PROPERTY_GET_SET(SourceS, Source)
  ;
public:
  /*override */virtual const char* Name() override { return "PTZ"; }
  /*override */virtual const char* ShortName() override { return "Z"; }
//protected:
//  /*override */virtual bool DoInit() override;
//  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;
//public:
//  /*override */virtual void Stop() override;

public:
  /*new */virtual bool GetAbilities(int& _AbilityFlag);
  /*new */virtual bool GetPosition(PtzVector& _Position);
  /*new */virtual bool GetRange(PtzVector& _MinPosition, PtzVector& _MaxPosition, PtzVector& _MinSpeed, PtzVector& _MaxSpeed);
  /*new */virtual bool GetHome(PtzVector& _HomePosition);
  /*new */virtual bool SetPosition(const PtzVector& _Position);
  /*new */virtual bool SetPosition(const PtzVector& _Position, const PtzVector& _Speed);
  /*new */virtual bool RelativeMove(const PtzVector& _Position);
  /*new */virtual bool RelativeMove(const PtzVector& _Position, const PtzVector& _Speed);
  /*new */virtual bool ContinuousMove(const PtzVector& _Speed, int _Timeout);
  /*new */virtual bool StopMove();
  /*new */virtual bool MoveHome();
  /*new */virtual bool MoveHome(const PtzVector& _Speed);
  /*new */virtual bool SetHome(const PtzVector& _Position);

public:
  static PtzS CreatePtz(SettingsA& settings);

protected:
  Ptz();
public:
  virtual ~Ptz();
};
