#pragma once

#include <QDir>

#include "PackLoaderA.h"


class PackLoaderFile: public PackLoaderA
{
  QDir mUpdateDir;

public:
  /*override */virtual bool LoadVer(QByteArray& ver) override;
  /*override */virtual bool LoadInfo(QByteArray& info) override;
  /*override */virtual bool LoadExternalsVer(QByteArray& ver) override;
  /*override */virtual bool LoadExternalsInfo(QByteArray& info) override;
  /*override */virtual bool LoadFile(const QString& path, QByteArray& data) override;

  /*override */virtual void SetCtrl(CtrlWorker* ctrl) override;
  /*override */virtual void Abort() override;

public:
  void SetPackPath(const QString& sourceBasePath);

public:
  PackLoaderFile();
  /*override */virtual ~PackLoaderFile();
};
