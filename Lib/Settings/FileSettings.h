#pragma once

#include <Lib/Include/Common.h>

#include "SettingsA.h"


DefineClassS(FileSettings);
DefineClassS(QSettings);

class FileSettings: public SettingsA
{
  QSettingsS mQSettings;

public:
  bool OpenLocal();
  bool OpenLocal(const QString& filename);
  bool OpenLocalWithId(int id);

public:
  /*override */virtual bool Open(const QString& path) Q_DECL_OVERRIDE;
  /*override */virtual bool Reload() Q_DECL_OVERRIDE;
  /*override */virtual bool Sync() Q_DECL_OVERRIDE;

  /*override */virtual bool BeginGroup(const QString& prefix) Q_DECL_OVERRIDE;
  /*override */virtual void EndGroup() Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetMandatoryValue(const QString& key, bool fatal = false) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) Q_DECL_OVERRIDE;
  /*override */virtual void SetValue(const QString& key, const QVariant& value) Q_DECL_OVERRIDE;

public:
  FileSettings();
  /*override */~FileSettings();
};

