#pragma once

#include <qsystemdetection.h>
#include <QElapsedTimer>
#include <QMap>

#include <Lib/Db/Db.h>
#include <Lib/Settings/SettingsA.h>
#include <LibV/Include/Frame.h>
#include <LibV/Include/Rect.h>
#include <LibV/Include/Region.h>

#include "Va.h"
#include "DebugWnd.h"


#ifdef Q_OS_WIN32
#define ANAL_DEBUG
#endif

DefineClassS(CtrlManager);
DefineClassS(DebugWnd);
DefineStructS(Detector);
DefineClassS(AnalyticsA);
DefineClassS(QByteArray);

/*abstract */class AnalyticsA
{
  PROPERTY_GET(int,          Id)
  FilesTableS                mFilesTable;
  VaStatTypeTableS           mVaStatTypeTable;
  VaStatTableS               mVaStatTable;
  VaStatHoursTableS          mVaStatHoursTable;
  VaStatDaysTableS           mVaStatDaysTable;

  PROPERTY_GET(ECompression, Compression)
  PROPERTY_GET(int,          Width)
  PROPERTY_GET(int,          Height)
  PROPERTY_GET(int,          Stride)
  bool                       mWarning;

  PROPERTY_GET(qint64,       StartTimestamp) // absolute time
  PROPERTY_GET(qint64,       LastTimestamp)  // absolute prev frame time
  PROPERTY_GET(qint64,       CurrentMs)      // relative to start time
  PROPERTY_GET(qint64,       FrameMs)        // relative to prev frame time
  PROPERTY_GET(qreal,        FrameSec)
  PROPERTY_GET(qint64,       CurrentFrame)

  PROPERTY_GET(bool,           UseStandBy)
  bool                         mStandByMode;
  QElapsedTimer                mSyncSettings;
  PROPERTY_GET_SET(SettingsAS, LocalSettings)

  const uchar*                 mImageData;
  PROPERTY_GET_SET(QByteArray, BackImage)
  bool                         mHasBackImage;
  PROPERTY_GET_SET(QByteArray, StatImage)
  qint64                       mStatTimestamp;
  qint64                       mStatMakeTimestamp;
  qint64                       mStatEndTimestamp;
  bool                         mStatDone;
  int                          mStatCreateMs;
  bool                         mUsingStats;

#ifdef ANAL_DEBUG
  DebugWndS                  mDebugWnd;
  QByteArray                 mDebugObj;
#endif

public:
  const uchar* SourceImageData() const { return mImageData; }
  void SetId(int _Id);
  void SetDb(const DbS& _Db);
  void SetSettings(const SettingsAS& _Settings);
  void SetDetectors(const QList<DetectorS>& _Detectors);
  void SetDebug(CtrlManager* _CtrlManager);

protected:
  /*new */virtual void InitSettings(const SettingsAS& _Settings) = 0;
  /*new */virtual void InitDetectors(const QList<DetectorS>& _Detectors) = 0;
  /*new */virtual bool InitImageParameters(int width, int height, int stride) = 0;

  /*new */virtual void AnalizePrepare(const uchar* imageData) = 0;
  /*new */virtual void AnalizeInit() = 0;
  /*new */virtual bool AnalizeStandBy() = 0;
  /*new */virtual bool AnalizeNormal() = 0;
  /*new */virtual void SaveVariables(const SettingsAS& settings) { Q_UNUSED(settings); }

  /*new */virtual bool GetStatAbbr(int type, QString& abbr);
  /*new */virtual int  GetStatTimeMs(int type);
  /*new */virtual bool GetStatImage(int type, QByteArray& image);
  /*new */virtual void ResetStat(int type);

  /*new */virtual int  GetDebugFrameCount();
  /*new */virtual bool GetDebugFrame(const int index, QString& text, EImageType& imageType, uchar* data, bool& save);

public:
  /*new */virtual bool HaveNextObject() = 0;
  /*new */virtual bool RetrieveNextObject(Object& object) = 0;
  /*new */virtual void Finish();

public:
  void Init();
  bool AnalizeFrame(Frame& frame); // return: true - has results, false - no results (init, reset, bad image, etc)

  bool CreateSnapshot(const Region<uchar>& region, QByteArray& data) const;
  bool CreateSnapshotNv12(const Region<uchar>& regionY, const Region<uchar>& regionUv, QByteArray& data) const;
  bool CreateImageNv12(const Region<uchar>& regionY, const Region<uchar>& regionUv, QImage& img) const;

protected:
  void SwitchStandByOn();
  void SwitchStandByOff();
  void SetCurrentAsBackImage();
  bool SyncBackImage();
  bool RestoreBackImage();

  QString GetVaFilename(const char* name) const;
  bool SyncImage(const char* name, const QByteArray& data) const;
  bool SyncImage(const char* name, const char* data, int dataSize) const;
  bool RestoreImage(const char* name, QByteArray& data) const;
  bool RestoreImage(const char* name, char* data, int dataSize) const;

protected:
  void GetDbgBackImage(uchar* data);
  void GetDbgSource(uchar* data);
  void GetDbgSourceWithLine(int line, uchar* data);
  void GetDbgSourceLine(int line, uchar* data);

private:
  void UpdateStats(const qint64& timestamp);
  void ResetStats();
  bool MakeStats();
  bool SaveStatImage(int statTypeId);
  void PrepareStatTimestamp(const qint64& timestamp);
  bool OnUpdateDimentions();
  bool Analize(const qint64& timestamp, const uchar* imageData);

  void Reset();
  void InitTime(const qint64& timestamp);
  bool UpdateTime(const qint64& timestamp);

  void SyncSettings();

public:
  AnalyticsA();
  /*new */virtual ~AnalyticsA();
};

