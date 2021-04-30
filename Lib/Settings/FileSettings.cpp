#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>

#include <Lib/Log/Log.h>
#include <Lib/Common/Var.h>
#include "FileSettings.h"


bool FileSettings::OpenLocal()
{
  QFileInfo appFile(QCoreApplication::applicationFilePath());
  return Open(GetVarFile(appFile.completeBaseName()));
}

bool FileSettings::OpenLocal(const QString& filename)
{
  return Open(GetVarFile(filename));
}

bool FileSettings::OpenLocalWithId(int id)
{
  QFileInfo appFile(QCoreApplication::applicationFilePath());
  return Open(GetVarFileWithId(appFile.completeBaseName(), id));
}

bool FileSettings::Open(const QString &path)
{
  mQSettings = QSettingsS(new QSettings(path, QSettings::IniFormat));
  mQSettings->setIniCodec("UTF-8");
  return mQSettings;
}

bool FileSettings::Reload()
{
  if (!mQSettings) {
    return false;
  }
  QString path = mQSettings->fileName();
  mQSettings = QSettingsS(new QSettings(path, QSettings::IniFormat));
  mQSettings->setIniCodec("UTF-8");
  return mQSettings;
}

bool FileSettings::Sync()
{
  mQSettings->sync();
  return true;
}

bool FileSettings::BeginGroup(const QString &prefix)
{
  mQSettings->beginGroup(prefix);
  return true;
}

void FileSettings::EndGroup()
{
  mQSettings->endGroup();
}

QVariant FileSettings::GetMandatoryValue(const QString &key, bool fatal)
{
  if (mQSettings) {
    QVariant value = mQSettings->value(key);
    if (value == QVariant()) {
      Log.Fatal(QString("Read setting '%1' fail").arg(key), fatal);
    } else {
      if (!IsSilent()) {
        Log.Info(QString("Read setting '%1' = '%2'").arg(key).arg(value.toString()));
      }
    }
    return value;
  } else {
    Log.Fatal("Settings not initialized");
    return QVariant();
  }
}

QVariant FileSettings::GetValue(const QString &key, const QVariant &defaultValue)
{
  if (mQSettings) {
    QVariant value = mQSettings->value(key);
    if (value == QVariant()) {
      value = defaultValue;
      if (IsAutoCreate()) {
        SetValue(key, value);
        if (!IsSilent()) {
          Log.Info(QString("Created setting '%1' = '%2'").arg(key).arg(value.toString()));
        }
      } else if (!IsSilent()) {
        Log.Warning(QString("Use default setting '%1' = '%2'").arg(key).arg(value.toString()));
      }
    } else {
      if (!IsSilent()) {
        Log.Info(QString("Read setting '%1' = '%2'").arg(key).arg(value.toString()));
      }
    }
    return value;
  } else {
    Log.Fatal(QString("Settings not initialized (query key: '%1')").arg(key));
    return defaultValue;
  }
}

void FileSettings::SetValue(const QString &key, const QVariant &value)
{
  if (mQSettings) {
    if (!IsSilent()) {
      Log.Info(QString("Set '%1' = '%2'").arg(key).arg(value.toString()));
    }
    return mQSettings->setValue(key, value);
  }
}

FileSettings::FileSettings()
{
}

FileSettings::~FileSettings()
{
}


