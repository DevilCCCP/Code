#include <QCoreApplication>
#include <QThread>
#include <QTemporaryDir>

#include <Lib/UpdaterCore/PackageCore.h>
#include <Lib/UpdaterCore/PackLoaderFile.h>
#include <Lib/UpdaterCore/PackLoaderHttp.h>

#include "BackgroundLoader.h"


bool BackgroundLoader::Init(const QString& uri, const QString& login, const QString& pass)
{
  if (mPackageLoader) {
    return true;
  }

  static const QString kFilePrefix("file://");
  static const QString kHttpPrefix("http");
  if (uri.startsWith(kFilePrefix)) {
    mPackageLoader.reset(new PackLoaderFile());
    mPackageLoader->setUri(uri.mid(kFilePrefix.size()));
  } else if (uri.startsWith(kHttpPrefix)) {
    mPackageLoader.reset(new PackLoaderHttp());
    mPackageLoader->setUri(uri);
  } else {
    return false;
  }
  mPackageLoader->setLogin(login);
  mPackageLoader->setPass(pass);

  return mLocalVersion.LoadFromThis();
}

bool BackgroundLoader::InitPack()
{
  if (!mPackage) {
    mTempDir.reset(new QTemporaryDir());
    if (!mTempDir->isValid()) {
      return false;
    }

    mPackage.reset(new PackageCore());
  }
  return true;
}

void BackgroundLoader::Check()
{
  if (mRunning || !mPackageLoader) {
    return;
  }

  mRunning = true;

  QByteArray verData;
  if (mPackageLoader->LoadVer(verData)) {
    Version upVer;
    upVer.LoadFromString(QString::fromUtf8(verData));
    if (upVer <= mLocalVersion) {
      emit CheckFinished(0);
    } else {
      emit CheckFinished(1);
    }
  } else {
    emit CheckFinished(-1);
  }

  mRunning = false;
}

void BackgroundLoader::Load()
{
  if (mRunning || !mPackageLoader) {
    return;
  }

  if (!InitPack()) {
    emit LoadFinished(QString());
    return;
  }

  mRunning = true;

  bool ok = mPackage->Deploy(mPackageLoader, mTempDir->path());
  emit LoadFinished(ok? mTempDir->path(): QString());

  mRunning = false;
}

void BackgroundLoader::Install()
{
  if (mRunning) {
    return;
  }

  mRunning = true;

  if (!mPackage->RunUpdate(mTempDir->path(), QCoreApplication::applicationDirPath())) {
    return;
  }
  mTempDir->setAutoRemove(false);

  emit InstallStarted();
}

void BackgroundLoader::Abort()
{
  mPackageLoader->Abort();
}

BackgroundLoader::BackgroundLoader(QObject* parent)
  : QObject(parent)
  , mRunning(false)
{
}

BackgroundLoader::~BackgroundLoader()
{
}
