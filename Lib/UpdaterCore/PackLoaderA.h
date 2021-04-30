#pragma once

#include <qsystemdetection.h>

#include <Lib/Include/Common.h>
#include <Lib/Common/Var.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Log/Log.h>


DefineClassS(PackLoaderA);
DefineClassS(CtrlWorker);

class PackLoaderA
{
  PROPERTY_GET_SET(QString,     Uri)
  PROPERTY_GET_SET(QString,     Login)
  PROPERTY_GET_SET(QString,     Pass)

  bool                          mInit;

public:
  /*new */virtual bool LoadVer(QByteArray& ver) = 0;
  /*new */virtual bool LoadInfo(QByteArray& info) = 0;
  /*new */virtual bool LoadExternalsVer(QByteArray& ver) = 0;
  /*new */virtual bool LoadExternalsInfo(QByteArray& info) = 0;
  /*new */virtual bool LoadFile(const QString& path, QByteArray& data) = 0;

  /*new */virtual void SetCtrl(CtrlWorker* ctrl) = 0;
  /*new */virtual void Abort() = 0;

public:
  bool FindPack();

public:
  PackLoaderA();
  /*new */virtual ~PackLoaderA();
};
