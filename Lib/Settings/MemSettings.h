#pragma once

#include <QMap>

#include <Lib/Include/Common.h>

#include "SettingsA.h"


DefineClassS(QSettings);
DefineClassS(MemSettings);

class MemSettings: public SettingsA
{
  QMap<QString, QVariant> mDictionary;
  QString                 mPrefix;

public:
  /*override */virtual bool Open(const QString&) override;
  /*override */virtual bool Sync() override;

  /*override */virtual bool BeginGroup(const QString& prefix) override;
  /*override */virtual void EndGroup() override;
  /*override */virtual QVariant GetMandatoryValue(const QString& key, bool fatal = false) override;
  /*override */virtual QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) override;
  /*override */virtual void SetValue(const QString& key, const QVariant& value) override;

public:
  MemSettings();
  /*override */~MemSettings();
};

