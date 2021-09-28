#pragma once

#include <QUrl>

#include "PackLoaderA.h"


DefineClassS(HttpUploaderSimple);

class PackLoaderHttpSimple: public PackLoaderA
{
  HttpUploaderSimpleS    mHttpUploaderSimple;

public:
  /*override */virtual bool LoadVer(QByteArray& ver) override;
  /*override */virtual bool LoadInfo(QByteArray& info) override;
  /*override */virtual bool LoadExternalsVer(QByteArray& ver) override;
  /*override */virtual bool LoadExternalsInfo(QByteArray& info) override;
  /*override */virtual bool LoadFile(const QString& path, QByteArray& data) override;

  /*override */virtual void SetCtrl(CtrlWorker* ctrl) override;
  /*override */virtual void Abort() override;

private:
  bool UploadFile(const QUrl& url, QByteArray& data, bool full = true);

public:
  PackLoaderHttpSimple();
  /*override */virtual ~PackLoaderHttpSimple();
};
