#pragma once

#include <QUrl>
#include <QElapsedTimer>

#include "PackLoaderA.h"


class QNetworkAccessManager;
class QNetworkReply;
class QSslError;
class QTimer;

class HttpUploaderSimple: public QObject
{
  QNetworkAccessManager*  mNetManager;
  QNetworkReply*          mRequestReply;
  QTimer*                 mTimeoutTimer;
  QElapsedTimer           mRequestTimer;

  QByteArray              mServerCert;

  Q_OBJECT

public:
  bool UploadFile(const QUrl& url, QByteArray& data, bool full = true);
  void Abort();

private:
  void LoadLocalCert();

signals:
  void UploadFinished(QByteArray data);

private slots:
  void OnTimeout();
  void OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
#ifndef QT_NO_SSL
  void OnSslErrors(const QList<QSslError>& errors);
#endif

public:
  HttpUploaderSimple();
  /*override */virtual ~HttpUploaderSimple();
};
