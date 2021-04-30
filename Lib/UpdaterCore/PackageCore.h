#pragma once

#include <QDir>
#include <QList>
#include <QByteArray>

#include <Lib/Include/Common.h>
#include <Lib/Common/Version.h>


DefineClassS(PackLoaderA);
DefineClassS(CtrlWorker);
DefineClassS(QCryptographicHash);

class PackageCore
{
  enum ECommand {
    eDir,
    eFile,
    eFileExec,
    eArchFile,
    eArchExec,
    eBatScript,
    eSqlScript,
    eIllegal
  };
  struct PackCommand {
    ECommand   Type;
    QString    Path;
    qint64     Size;
    Version    FileVersion;
    QByteArray FileHash;
    bool       DeployDone;
    bool       ExecDone;

    PackCommand(ECommand _Type, const QString& _Path)
      : Type(_Type), Path(_Path), Size(0), DeployDone(false), ExecDone(false)
    { }
    PackCommand(ECommand _Type, const QString& _Path, qint64 _Size, const Version& _Ver, const QByteArray& _Hash)
      : Type(_Type), Path(_Path), Size(_Size), FileVersion(_Ver), FileHash(_Hash), DeployDone(false), ExecDone(false)
    { }
  };

  enum EMode {
    eModePrepare,
    eModeDeploy,
    eModeInstallSoft,
    eModeInstallAggressive,
    eModeUpdate,
    eModeIllegal
  };

  PackLoaderAS        mPackLoader;
  CtrlWorker*         mCtrl;
  QList<PackCommand>  mCommands;
  QList<PackCommand>  mCommandsExt;

  QDir                mDestDir;
  EMode               mPackingMode;
  QString             mDestPath;
  QString             mCurrentPath;
  QCryptographicHashS mMd5;

  PROPERTY_GET(Version,    PackVersion)
  PROPERTY_GET(Version,    ExternalsVersion)
  PROPERTY_GET(QString,    InstallerPath)
  PROPERTY_GET(QByteArray, Md5Hash)
  ;
public:
  void SetCtrl(CtrlWorker* _Ctrl) { mCtrl = _Ctrl; }

public:
  bool Prepare(const QString& sourceBasePath, const QString& destBasePath);
  bool Install(const QString& sourceBasePath, const QString& destBasePath, bool aggressive);
  bool Update(const QString& sourceBasePath, const QString& destBasePath);
  bool Deploy(const PackLoaderAS& packLoader, const QString& destBasePath);
  bool RunUpdate(const QString& sourceBasePath, const QString& destBasePath);

private:
  bool DeployWithInfo(const QByteArray& info, const QString& destBasePath);
  bool DeployWithInfoExt(const QByteArray& info, const QString& destBasePath);
  bool DeployPack(const QString& destBasePath);
  bool DeployOne(PackCommand* cmd);
  bool DeployDir(const QString& path);
  bool DeployFile(const QString& path, bool arch, bool exec);
  bool ExecScripts();
  bool ExecFile(const QString& scriptPath);

protected:
  /*new */virtual bool ExecSql(const QString& scriptPath);
  /*new */virtual bool ConnectDb();

private:
  bool WriteFile(const QString& path, const QByteArray& data);
  bool ChmodFile(const QString& path);
  bool HashFile(const QString& path);

  bool PrepareInfo(QByteArray& info);

  bool ValidateHash();

public:
  bool ParseInfo(const QList<QByteArray>& lines);
  bool ParseInfoExt(const QList<QByteArray>& lines);

private:
  bool ParseLine(const QByteArray& line);
  bool ParseLineExt(const QByteArray& line);
  QByteArray GetInfoData() const;
  QString ModeToString();

public:
  PackageCore();
  virtual ~PackageCore();
};
