#include <QCoreApplication>
#include <QSettings>

#include "Version.h"


QString Version::mLocalVersion;

bool Version::InitLocalVersion()
{
  Version ver;
  if (!ver.LoadFromThis()) {
    return false;
  }
  mLocalVersion = ver.ToString();
  return true;
}

QString Version::LocalVersion()
{
  return mLocalVersion;
}

bool Version::LoadFromString(const QString& textVersion)
{
  QStringList versionList = textVersion.split('.');
  mMajorVersion = (versionList.size() > 0)? versionList[0].toInt(): 0;
  mMinorVersion = (versionList.size() > 1)? versionList[1].toInt(): 0;
  mRevision     = (versionList.size() > 2)? versionList[2].toInt(): 0;
  return true;
}

bool Version::LoadFromPack(const QString& packPath)
{
  QSettings verSettings(packPath + "/Version.ini", QSettings::IniFormat);
  verSettings.setIniCodec("UTF-8");
  return LoadFromString(verSettings.value("Ver").toString());
}

bool Version::LoadFromThis()
{
  return LoadFromPack(QCoreApplication::applicationDirPath());
}

QString Version::ToString() const
{
  return QString("%1.%2.%3").arg(mMajorVersion).arg(mMinorVersion).arg(mRevision);
}

QString Version::ToFilename() const
{
  return QString("%1_%2_%3").arg(mMajorVersion).arg(mMinorVersion).arg(mRevision);
}

bool Version::IsEmpty() const
{
  return mMajorVersion == 0 && mMinorVersion == 0 && mRevision == 0;
}

bool Version::Eq(const Version& other) const
{
  return mMajorVersion == other.mMajorVersion && mMinorVersion == other.mMinorVersion && mRevision == other.mRevision;
}

bool Version::Less(const Version& other) const
{
  if (mMajorVersion < other.mMajorVersion) {
    return true;
  } else if (mMajorVersion == other.mMajorVersion) {
    if (mMinorVersion < other.mMinorVersion) {
      return true;
    } else if (mMinorVersion == other.mMinorVersion) {
      if (mRevision < other.mRevision) {
        return true;
      }
    }
  }
  return false;
}


Version::Version()
  : mMajorVersion(0), mMinorVersion(0), mRevision(0)
{
}

