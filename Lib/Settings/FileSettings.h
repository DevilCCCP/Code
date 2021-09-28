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
  /*override */virtual bool Open(const QString& path) override;
  /*override */virtual bool Reload() override;
  /*override */virtual bool Sync() override;

  /*override */virtual bool BeginGroup(const QString& prefix) override;
  /*override */virtual void EndGroup() override;
  /*override */virtual QVariant GetMandatoryValue(const QString& key, bool fatal = false) override;
  /*override */virtual QVariant GetValue(const QString& key, const QVariant& defaultValue = QVariant()) override;
  /*override */virtual void SetValue(const QString& key, const QVariant& value) override;

public:
  FileSettings();
  /*override */~FileSettings();
};

