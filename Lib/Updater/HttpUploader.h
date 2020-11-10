#pragma once

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QMutex>
#include <QUrl>

#include "PackLoaderA.h"


DefineClassS(QNetworkAccessManager);
DefineClassS(QEventLoop);
DefineClassS(QTimer);

class HttpUploader: public QObject
{
  QNetworkAccessManager*  mNetManager;
  QEventLoop*             mEventLoop;
  QTimer*                 mTimeoutTimer;
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
