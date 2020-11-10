#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>

#include <Lib/Log/Log.h>

#include "PackLoaderHttp.h"
#include "HttpUploader.h"


const int kRetryCount = 3;

bool PackLoaderHttp::LoadVer(QByteArray& ver)
{
  QUrl url(getUri() + ".info");
  if (!url.isValid()) {
    if (!mFatal) {
      Log.Fatal(QString("Url not valid ('%1')").arg(getUri()));
      mFatal = true;
    }
    return false;
  }

  return mHttpUploader->UploadFile(url, ver, false);
}

bool PackLoaderHttp::LoadInfo(QByteArray& info)
{
  return mHttpUploader->UploadFile(QUrl(getUri() + ".info"), info, true);
}

bool PackLoaderHttp::LoadFile(const QString& path, QByteArray& data)
{
  QString fullpath = getUri() + (path.startsWith('.')? (path.startsWith("./")? path.mid(2): path.mid(1)): path);
  for (int i = 0; i < kRetryCount; i++) {
    if (mHttpUploader->UploadFile(QUrl(fullpath), data)) {
      return true;
    } else if (!mHttpUploader->Timeout()) {
      break;
    } else {
      Log.Info(QString("Retry (%1 / %2)").arg(i + 1).arg(kRetryCount));
    }
  }
  return false;
}

void PackLoaderHttp::SetCtrl(CtrlWorker* ctrl)
{
  mHttpUploader->SetCtrl(ctrl);
}

void PackLoaderHttp::Abort()
{
  mHttpUploader->Abort();
}


PackLoaderHttp::PackLoaderHttp()
  : mHttpUploader(new HttpUploader())
  , mFatal(false)
{
}

PackLoaderHttp::~PackLoaderHttp()
{
}
