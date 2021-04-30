#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QFile>
#include <QDebug>

#include "HttpUploaderSimple.h"


const int kRequestTimeoutMs = 30000;
const int kTimerMs = 500;

bool HttpUploaderSimple::UploadFile(const QUrl& url, QByteArray& data, bool full)
{
  if (!mNetManager) {
    mNetManager = new QNetworkAccessManager();
    mTimeoutTimer = new QTimer();
    mTimeoutTimer->setInterval(kTimerMs);

    connect(mTimeoutTimer, &QTimer::timeout, this, &HttpUploaderSimple::OnTimeout);
  }

  QNetworkRequest request(url);
  request.setRawHeader("Connection", "Close");
  qDebug() << "Request get";
  mRequestReply = mNetManager->get(request);

  connect(mRequestReply, &QNetworkReply::downloadProgress, this, &HttpUploaderSimple::OnDownloadProgress);
#ifndef QT_NO_SSL
  connect(mRequestReply, &QNetworkReply::sslErrors, this, &HttpUploaderSimple::OnSslErrors, Qt::DirectConnection);
#endif

  mTimeoutTimer->start();
  mRequestTimer.start();
  QEventLoop loop;
  connect(mRequestReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();
  mTimeoutTimer->stop();

  qDebug() << "Request finished" << mRequestReply->error() << mRequestReply->errorString();

  data.clear();
  data = full? mRequestReply->readAll(): mRequestReply->readLine();
  mRequestReply->deleteLater();

  return !data.isEmpty();
}

void HttpUploaderSimple::Abort()
{
  if (mRequestReply) {
    mRequestReply->abort();
  }
}

void HttpUploaderSimple::LoadLocalCert()
{
  if (!mServerCert.isEmpty()) {
    return;
  }

  QFile filePem(":/Cert/server.pem");
  if (filePem.open(QFile::ReadOnly)) {
    mServerCert = filePem.readAll();
    mServerCert.replace("\r", "");
    mServerCert.replace("\n", "");
  }
}

void HttpUploaderSimple::OnTimeout()
{
  if (mRequestTimer.elapsed() > kRequestTimeoutMs) {
    Abort();
  }
}

void HttpUploaderSimple::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_UNUSED(bytesReceived);
  Q_UNUSED(bytesTotal);

  mRequestTimer.restart();
}

#ifndef QT_NO_SSL
void HttpUploaderSimple::OnSslErrors(const QList<QSslError>& errors)
{
  if (sender()) {
    qDebug() << "SSL on errors" << sender()->objectName();
  } else {
    qDebug() << "SSL on errors ???";
  }

  Q_UNUSED(errors);
  for (const QSslError& error: errors) {
    qDebug() << error.errorString();
  }

  if (mRequestReply) {
    const QSslCertificate& cert = mRequestReply->sslConfiguration().peerCertificate();
    QByteArray certPem = cert.toPem();
    certPem.replace("\r", "");
    certPem.replace("\n", "");
    LoadLocalCert();
    if (certPem == mServerCert) {
      mRequestReply->ignoreSslErrors();
    }
  } else {
    qDebug() << "not checked";
  }
}
#endif


HttpUploaderSimple::HttpUploaderSimple()
  : mNetManager(nullptr), mRequestReply(nullptr), mTimeoutTimer(nullptr)
{
}

HttpUploaderSimple::~HttpUploaderSimple()
{
}
