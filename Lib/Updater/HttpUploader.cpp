#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QMutexLocker>
#include <QFile>

#include <Lib/Log/Log.h>

#include "HttpUploader.h"


const int kRequestTimeoutMs = 30000;

bool HttpUploader::UploadFile(const QUrl& url, QByteArray& data, bool full)
{
  if (!mNetManager) {
    mNetManager = new QNetworkAccessManager();
    mEventLoop = new QEventLoop();
    mTimeoutTimer = new QTimer();
    mTimeoutTimer->setInterval(kRequestTimeoutMs);

    connect(mTimeoutTimer, &QTimer::timeout, this, &HttpUploader::OnTimeout);
    connect(this, &HttpUploader::DoAbort, mEventLoop, &QEventLoop::quit, Qt::QueuedConnection);
  }

  QNetworkRequest request(url);
  request.setRawHeader("Connection", "Close");
  QNetworkReply* reply = mNetManager->get(request);

  connect(reply, &QNetworkReply::downloadProgress, mTimeoutTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
#ifndef QT_NO_SSL
  connect(reply, &QNetworkReply::sslErrors, this, &HttpUploader::OnSslErrors);
#endif
  connect(reply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);

  mTimeout = false;
  mTimeoutTimer->start();
  Log.Trace(QString("Connecting ..."));
  if (!HasAbort()) {
    mEventLoop->exec();
  }
  Log.Trace(QString("Done"));
  mTimeoutTimer->stop();

  data.clear();
  bool ok = !mTimeout && reply->error() == QNetworkReply::NoError;
  if (ok) {
    data = full? reply->readAll(): reply->readLine();
  }
  if (ok && !data.isEmpty()) {
    mError = 0;
  } else {
    if (mTimeout) {
      if (mError != 1) {
        Log.Warning(QString("Request timeout"));
        mError = 1;
      }
    } else if (!ok) {
      if (mError != 2) {
        Log.Warning(QString("Request fail (code: %1, err: '%2')").arg((int)reply->error()).arg(reply->errorString()));
        mError = 2;
      }
    } else {
      if (mError != 3) {
        Log.Warning(QString("Data read fail"));
        mError = 3;
      }
      Reset();
    }
  }
  reply->deleteLater();

  return !data.isEmpty();
}

void HttpUploader::Abort()
{
  Log.Trace("Connection cancel");
  SetAbort();
  mError = true;

  emit DoAbort();
}

void HttpUploader::LoadLocalCert()
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

void HttpUploader::SetAbort()
{
  QMutexLocker lock(&mMutex);
  mAbort = true;
}

bool HttpUploader::HasAbort()
{
  QMutexLocker lock(&mMutex);
  return mAbort;
}

void HttpUploader::Reset()
{
  if (mNetManager) {
    mNetManager->deleteLater();
    mNetManager = nullptr;
    mEventLoop->deleteLater();
    mEventLoop = nullptr;
    mTimeoutTimer->deleteLater();
    mTimeoutTimer = nullptr;
  }
}

void HttpUploader::OnTimeout()
{
  mTimeout = true;
  emit DoAbort();
}

#ifndef QT_NO_SSL
void HttpUploader::OnSslErrors(const QList<QSslError>& errors)
{
  Q_UNUSED(errors);

  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (reply) {
    const QSslCertificate& cert = reply->sslConfiguration().peerCertificate();
    QByteArray certPem = cert.toPem();
    certPem.replace("\r", "");
    certPem.replace("\n", "");
    LoadLocalCert();
    if (certPem != mServerCert) {
      LOG_ERROR_ONCE(QString("SSL untrusted certificate: %1").arg(cert.toPem().constData()));
      return;
    }
    Log.Trace(QString("Peer use trusted cert"));
    reply->ignoreSslErrors();
  }
}
#endif


HttpUploader::HttpUploader()
  : mNetManager(nullptr), mEventLoop(nullptr), mTimeoutTimer(nullptr)
  , mError(0), mTimeout(false), mAbort(false)
{
}

HttpUploader::~HttpUploader()
{
  Reset();
}
