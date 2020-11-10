#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "PackLoaderFile.h"


bool PackLoaderFile::LoadVer(QByteArray& ver)
{
  mUpdateDir = QDir(getUri());

  QFile file(mUpdateDir.absoluteFilePath(".info"));
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }
  ver = file.readLine();
  return !ver.isEmpty();
}

bool PackLoaderFile::LoadInfo(QByteArray& info)
{
  mUpdateDir = QDir(getUri());

  QFile file(mUpdateDir.absoluteFilePath(".info"));
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }
  info = file.readAll();
  return file.error() == QFileDevice::NoError && !info.isEmpty();
}

bool PackLoaderFile::LoadFile(const QString& path, QByteArray& data)
{
  QFile file(mUpdateDir.absoluteFilePath(path));
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }
  data = file.readAll();
  return file.error() == QFileDevice::NoError;
}

void PackLoaderFile::SetCtrl(CtrlWorker* ctrl)
{
  Q_UNUSED(ctrl);
}

void PackLoaderFile::Abort()
{
}

void PackLoaderFile::SetPackPath(const QString& sourceBasePath)
{
  mUpdateDir.setPath(sourceBasePath);
}


PackLoaderFile::PackLoaderFile()
{
}

PackLoaderFile::~PackLoaderFile()
{
}
