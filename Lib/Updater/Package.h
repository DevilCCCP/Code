#pragma once

#include <QDir>
#include <QList>
#include <QByteArray>

#include <Lib/Include/Common.h>
#include <Lib/Common/Version.h>


DefineClassS(PackLoaderA);
DefineClassS(Db);
DefineClassS(QCryptographicHash);

class Package
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
    ECommand Type;
    QString  Path;
    bool     DeployDone;
    bool     ExecDone;

    PackCommand(ECommand _Type, const QString& _Path): Type(_Type), Path(_Path), DeployDone(false), ExecDone(false) { }
  };

  enum EMode {
    eModePrepare,
    eModeDeploy,
    eModeInstallSoft,
    eModeInstallAggressive,
    eModeIllegal
  };

  PackLoaderAS        mPackLoader;
  QList<PackCommand>  mCommands;
  DbS                 mDb;

  QDir                mDestDir;
  EMode               mPackingMode;
  QString             mDestPath;
  QString             mCurrentPath;
  QCryptographicHashS mMd5;

  PROPERTY_GET(Version,    PackVersion)
  PROPERTY_GET(QString,    InstallerPath)
  PROPERTY_GET(QByteArray, Md5Hash)
  ;
public:
  bool Prepare(const QString& sourceBasePath, const QString& destBasePath);
  bool Install(const QString& sourceBasePath, const QString& destBasePath, bool aggressive);
  bool Deploy(const PackLoaderAS& packLoader, const QString& destBasePath);
private:
  bool DeployWithInfo(const QByteArray& info, const QString& destBasePath);
  bool DeployPack(const QString& destBasePath);
  bool DeployOne(PackCommand* cmd);
  bool DeployDir(const QString& path);
  bool DeployFile(const QString& path, bool arch, bool exec);
  bool ExecScripts();
  bool ExecFile(const QString& path);
  bool ExecSql(const QString& path);
  bool ConnectDb();

  bool WriteFile(const QString& path, const QByteArray& data);
  bool ChmodFile(const QString& path);
  bool HashFile(const QString& path);

  bool PrepareInfo(QByteArray& info);

  bool ValidateHash();

public:
  bool ParseInfo(const QList<QByteArray>& lines);

private:
  bool ParseLine(const QByteArray& line);
  QByteArray GetInfoData() const;
  QString ModeToString();

public:
  Package();
};
