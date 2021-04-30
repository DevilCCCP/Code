#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>

#include "PackLoaderHttpSimple.h"
#include "HttpUploaderSimple.h"


const int kRetryCount = 3;

bool PackLoaderHttpSimple::LoadVer(QByteArray& ver)
{
  QUrl url(getUri() + ".info");
  if (!url.isValid()) {
    return false;
  }

  return mHttpUploaderSimple->UploadFile(url, ver, false);
}

bool PackLoaderHttpSimple::LoadInfo(QByteArray& info)
{
  return mHttpUploaderSimple->UploadFile(QUrl(getUri() + ".info"), info, true);
}

bool PackLoaderHttpSimple::LoadExternalsVer(QByteArray& ver)
{
  QUrl url(getUri() + ".externals");
  return mHttpUploaderSimple->UploadFile(url, ver, false);
}

bool PackLoaderHttpSimple::LoadExternalsInfo(QByteArray& info)
{
  return mHttpUploaderSimple->UploadFile(QUrl(getUri() + ".externals"), info, true);
}

bool PackLoaderHttpSimple::LoadFile(const QString& path, QByteArray& data)
{
  QString fullpath = getUri() + (path.startsWith('.')? (path.startsWith("./")? path.mid(2): path.mid(1)): path);
  for (int i = 0; i < kRetryCount; i++) {
    if (mHttpUploaderSimple->UploadFile(QUrl(fullpath), data)) {
      return true;
    }
  }
  return false;
}

void PackLoaderHttpSimple::SetCtrl(CtrlWorker* ctrl)
{
  Q_UNUSED(ctrl);
}

void PackLoaderHttpSimple::Abort()
{
  mHttpUploaderSimple->Abort();
}


PackLoaderHttpSimple::PackLoaderHttpSimple()
  : mHttpUploaderSimple(new HttpUploaderSimple())
{
}

PackLoaderHttpSimple::~PackLoaderHttpSimple()
{
}
