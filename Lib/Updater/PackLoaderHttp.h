#pragma once

#include <QUrl>

#include "PackLoaderA.h"


DefineClassS(HttpUploader);

class PackLoaderHttp: public PackLoaderA
{
  HttpUploaderS          mHttpUploader;

  bool                   mFatal;

public:
  /*override */virtual bool LoadVer(QByteArray& ver) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadInfo(QByteArray& info) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadFile(const QString& path, QByteArray& data) Q_DECL_OVERRIDE;

  /*override */virtual void SetCtrl(CtrlWorker* ctrl) Q_DECL_OVERRIDE;
  /*override */virtual void Abort() Q_DECL_OVERRIDE;

private:
  bool UploadFile(const QUrl& url, QByteArray& data, bool full = true);

public:
  PackLoaderHttp();
  /*override */virtual ~PackLoaderHttp();
};
