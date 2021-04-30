#pragma once

#include <QSharedPointer>
#include <QVariant>

#include <Lib/Include/Common.h>


DefineClassS(SettingsA);
DefineClassS(QSettings);

/*abstract */class SettingsA
{
  bool mSilent;
  bool mAutoCreate;

public:
  void SetSilent(bool _Silent) { mSilent = _Silent; }
  bool IsSilent() { return mSilent; }
  void SetAutoCreate(bool _AutoCreate) { mAutoCreate = _AutoCreate; }
  bool IsAutoCreate() { return mAutoCreate; }

public:
  /*new */virtual bool Open(const QString& path) = 0;
  /*new */virtual bool Reload() { return true; }
  /*new */virtual bool Sync() = 0;

  /*new */virtual bool BeginGroup(const QString & prefix) = 0;
  /*new */virtual void EndGroup() = 0;
  /*new */virtual QVariant GetMandatoryValue(const QString & key, bool fatal = false) = 0;
  /*new */virtual QVariant GetValue(const QString & key, const QVariant & defaultValue = QVariant()) = 0;
  /*new */virtual void SetValue(const QString & key, const QVariant & value) = 0;

public:
  SettingsA(bool _Silent = false)
    : mSilent(_Silent)
  { }
  /*new */virtual ~SettingsA() { }
};

