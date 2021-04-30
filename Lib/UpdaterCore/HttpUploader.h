#pragma once

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QMutex>
#include <QUrl>
#include <QElapsedTimer>

#include "PackLoaderA.h"


class QNetworkAccessManager;
class QEventLoop;
class QNetworkReply;
class QTimer;

class HttpUploader: public QObject
{
  QEventLoop*             mEventLoop;
  QNetworkAccessManager*  mNetManager;
  QNetworkReply*          mNetReply;
  QTimer*                 mTimeoutTimer;
  QElapsedTimer           mRequestTimer;
  QMutex                  mMutex;
  CtrlWorker*             mCtrl;

  QByteArray              mServerCert;
  int                     mError;
  bool                    mTimeout;
  bool                    mAbort;

  Q_OBJECT

public:
  bool Timeout() const { return mTimeout; }

public:
  bool UploadFile(const QUrl& url, QByteArray& data, bool full = true);
  void SetCtrl(CtrlWorker* ctrl);
  void Abort();

private:
  void LoadLocalCert();

  void SetAbort();
  bool HasAbort();
  void Reset();

signals:
  void DoAbort();

private slots:
  void OnTimeout();
  void OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
#ifndef QT_NO_SSL
  void OnSslErrors(const QList<QSslError>& errors);
#endif

public:
  HttpUploader();
  /*override */virtual ~HttpUploader();
};
