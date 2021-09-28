#pragma once

#include <QMutex>
#include <QWaitCondition>

#include "../Ptz.h"


DefineClassS(PtzOnvif);
DefineClassS(QNetworkAccessManager);
DefineClassS(QEventLoop);
DefineClassS(QXmlStreamReader);

class PtzOnvif: public Ptz
{
  QString                mUri;
  QString                mPtzUri;
  bool                   mFlipVert;
  bool                   mFlipHorz;
  qreal                  mZoomScale;
  QString                mLogin;
  QString                mPassword;
  QByteArray             mNonce;
  bool                   mInit;

  QByteArray             mGetCapabilitiesXml;
  QByteArray             mGetProfilesXml;
  QByteArray             mGetStatusXml;
  QByteArray             mAbsoluteMoveXml;
  QByteArray             mContinuousMoveXml;
  QByteArray             mRelativeMoveXml;
  QByteArray             mStopXml;
  QByteArray             mVectorXml;
  QByteArray             mSecurityXml;

  QNetworkAccessManager* mNetManager;
  QEventLoop*            mEventLoop;
  QMutex                 mCmdMutex;
  QWaitCondition         mCmdWait;

  QByteArray             mProfileToken;
  PtzVector              mMinPosition;
  PtzVector              mMaxPosition;
  PtzVector              mMinSpeed;
  PtzVector              mMaxSpeed;
  PtzVector              mHomePosition;

  QByteArray             mResultXml;
  QXmlStreamReaderS      mXmlReaderS;
  QXmlStreamReader*      mXmlReader;

protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;
public:
//  /*override */virtual void Stop() override;

public:
  /*override */virtual bool GetAbilities(int& _AbilityFlag) override;
  /*override */virtual bool GetPosition(PtzVector& _Position) override;
  /*override */virtual bool GetRange(PtzVector& _MinPosition, PtzVector& _MaxPosition, PtzVector& _MinSpeed, PtzVector& _MaxSpeed) override;
  /*override */virtual bool GetHome(PtzVector& _HomePosition) override;
  /*override */virtual bool SetPosition(const PtzVector& _Position) override;
  /*override */virtual bool SetPosition(const PtzVector& _Position, const PtzVector& _Speed) override;
  /*override */virtual bool RelativeMove(const PtzVector& _Position) override;
  /*override */virtual bool RelativeMove(const PtzVector& _Position, const PtzVector& _Speed) override;
  /*override */virtual bool ContinuousMove(const PtzVector& _Speed, int _Timeout) override;
  /*override */virtual bool StopMove() override;
  /*override */virtual bool MoveHome() override;
  /*override */virtual bool MoveHome(const PtzVector& _Speed) override;
  /*override */virtual bool SetHome(const PtzVector& _Position) override;

private:
  bool InitPtz();
  bool PrepareMove();
  bool TakeRespond(const char* name);
  bool TakeElement(const char* name, bool warn = false);
  bool ParseCapabilities();
  bool ParseProfiles();
  bool ParseLimits();
  bool ParseStatus(PtzVector& position);
  bool ParsePosition(PtzVector& position);
  bool ParseRanges(float* minX, float* maxX, float* minY = nullptr, float* maxY = nullptr);
  bool ParseRange(float* minX, float* maxX);

  QByteArray GetXmlVector(const char* name, const PtzVector& vect, bool relative);
  QByteArray GetXmlTimeout(int timeout);
  bool GetCapabilities();
  bool GetConfigurations();
  QByteArray AddSecurity(const QByteArray& xml);
  bool SendXml();
  bool SendCommand(const QString& cmd, const QByteArray& xml, bool ptz);

  bool ReadResourceFile(const char* fileName, QByteArray& fileData);

  float CoordToLocal(const QStringRef& text, float valueMin, float valueMax, float valueDefault, bool* ok = nullptr);
  float CoordToCamera(float value, float valueMin, float valueMax, bool relative);
  float CoordToCameraAbsolut(float value, float valueMin, float valueMax);
  float CoordToCameraRelative(float value, float valueMin, float valueMax);

private:
  void OnCommandFinish();

public:
  PtzOnvif(SettingsA& settings);
  virtual ~PtzOnvif();
};
