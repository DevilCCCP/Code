#include <Lib/Log/Log.h>

#include "MemSettings.h"


bool MemSettings::Open(const QString &)
{
  return true;
}

bool MemSettings::Sync()
{
  return false;
}

bool MemSettings::BeginGroup(const QString &prefix)
{
  mPrefix += prefix + "/";
  return true;
}

void MemSettings::EndGroup()
{
  int pos = mPrefix.lastIndexOf("/", -2);
  if (pos > 0) {
    mPrefix = mPrefix.left(pos);
  } else {
    mPrefix.clear();
  }
}

QVariant MemSettings::GetMandatoryValue(const QString &key, bool fatal)
{
  auto itr = mDictionary.find(key);
  QVariant value;
  if (itr == mDictionary.end()) {
    Log.Fatal(QString("Read setting '%1' fail").arg(key), fatal);
  } else {
    value = itr.value();
    Log.Info(QString("Read setting '%1' = '%2'").arg(key).arg(value.toString()));
  }
  return value;
}

QVariant MemSettings::GetValue(const QString &key, const QVariant &defaultValue)
{
  auto itr = mDictionary.find(key);
  QVariant value;
  if (itr == mDictionary.end()) {
    value = defaultValue;
    Log.Warning(QString("Use default setting '%1' = '%2'").arg(key).arg(value.toString()));
  } else {
    value = itr.value();
    Log.Info(QString("Read setting '%1' = '%2'").arg(key).arg(value.toString()));
  }
  return value;
}

void MemSettings::SetValue(const QString &key, const QVariant &value)
{
  mDictionary[key] = value;
}


MemSettings::MemSettings()
{
}

MemSettings::~MemSettings()
{
}
