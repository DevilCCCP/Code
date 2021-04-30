#pragma once

#include <QObject>

#include <Lib/Common/Version.h>


DefineClassS(PackLoaderA);
DefineClassS(PackageCore);
DefineClassS(QTemporaryDir);

class BackgroundLoader: public QObject
{
  PackLoaderAS      mPackageLoader;
  PackageCoreS      mPackage;
  QTemporaryDirS    mTempDir;
  Version           mLocalVersion;
  bool              mRunning;

  Q_OBJECT

public:
  bool Init(const QString& uri, const QString& login, const QString& pass);

private:
  bool InitPack();

public slots:
  void Check();
  void Load();
  void Install();
  void Abort();

signals:
  void CheckFinished(int version);
  void LoadFinished(QString path);
  void InstallStarted();

public:
  explicit BackgroundLoader(QObject* parent = nullptr);
  virtual ~BackgroundLoader();
};
