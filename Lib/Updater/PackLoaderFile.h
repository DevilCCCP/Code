#pragma once

#include <QDir>

#include "PackLoaderA.h"


class PackLoaderFile: public PackLoaderA
{
  QDir mUpdateDir;

public:
  /*override */virtual bool LoadVer(QByteArray& ver) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadInfo(QByteArray& info) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadFile(const QString& path, QByteArray& data) Q_DECL_OVERRIDE;

  /*override */virtual void SetCtrl(CtrlWorker* ctrl) Q_DECL_OVERRIDE;
  /*override */virtual void Abort() Q_DECL_OVERRIDE;

public:
  void SetPackPath(const QString& sourceBasePath);

public:
  PackLoaderFile();
  /*override */virtual ~PackLoaderFile();
};
