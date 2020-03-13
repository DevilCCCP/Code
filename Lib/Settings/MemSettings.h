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
  /*override */virtual bool Open(const QString&) Q_DECL_OVERRIDE;
  /*override */virtual bool Sync() Q_DECL_OVERRIDE;

  /*override */virtual bool BeginGroup(const QString& prefix) Q_DECL_OVERRIDE;
  /*override */virtual void EndGroup() Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetMandatoryValue(const QString& key, bool fatal = false) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) Q_DECL_OVERRIDE;
  /*override */virtual void SetValue(const QString& key, const QVariant& value) Q_DECL_OVERRIDE;

public:
  MemSettings();
  /*override */~MemSettings();
};

