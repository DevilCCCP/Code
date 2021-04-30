#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QMutexLocker>
#include <QFile>

#include <Lib/Ctrl/CtrlWorker.h>
#include <Lib/Log/Log.h>

#include "HttpUploader.h"


const int kRequestTimeoutMs = 30000;
const int kTimerMs = 500;

bool HttpUploader::UploadFile(const QUrl& url, QByteArray& data, bool full)
{
  if (!mEventLoop) {
    mEventLoop = new QEventLoop();
    mNetManager = new QNetworkAccessManager();
    mTimeoutTimer = new QTimer();
    mTimeoutTimer->setInterval(kTimerMs);

    connect(mTimeoutTimer, &QTimer::timeout, this, &HttpUploader::OnTimeout);
    connect(this, &HttpUploader::DoAbort, mEventLoop, &QEventLoop::quit);
  }

  QNetworkRequest request(url);
  request.setRawHeader("Connection", "Close");
  mNetReply = mNetManager->get(request);

  connect(mNetReply, &QNetworkReply::downloadProgress, this, &HttpUploader::OnDownloadProgress);
#ifndef QT_NO_SSL
  connect(mNetReply, &QNetworkReply::sslErrors, this, &HttpUploader::OnSslErrors, Qt::DirectConnection);
#endif
  connect(mNetReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);

  mTimeout = false;
  mTimeoutTimer->start();
  mRequestTimer.start();
  Log.Trace(QString("Connecting ..."));
  if (!HasAbort()) {
    mEventLoop->exec();
  }
  Log.Trace(QString("Done"));
  mTimeoutTimer->stop();

  data.clear();
  bool ok = !mTimeout && mNetReply->error() == QNetworkReply::NoError;
  if (ok) {
    data = full? mNetReply->readAll(): mNetReply->readLine();
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
        Log.Warning(QString("Request fail (code: %1, err: '%2')").arg((int)mNetReply->error()).arg(mNetReply->errorString()));
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
  mNetReply->deleteLater();
  mNetReply = nullptr;

  return !data.isEmpty();
}

void HttpUploader::SetCtrl(CtrlWorker* ctrl)
{
  mCtrl = ctrl;
}

void HttpUploader::Abort()
{
  if (mEventLoop && mEventLoop->isRunning()) {
    Log.Trace("Connection cancel");
    SetAbort();
    mError = true;

    emit DoAbort();
  }
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
  if (mEventLoop) {
    mEventLoop->deleteLater();
    mEventLoop = nullptr;
    mNetManager->deleteLater();
    mNetManager = nullptr;
    mTimeoutTimer->deleteLater();
    mTimeoutTimer = nullptr;
  }
}

void HttpUploader::OnTimeout()
{
  if (mRequestTimer.elapsed() > kRequestTimeoutMs) {
    mTimeout = true;
    emit DoAbort();
  }
}

void HttpUploader::OnDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_UNUSED(bytesReceived);
  Q_UNUSED(bytesTotal);

  if (mCtrl) {
    if (!mCtrl->WorkStep()) {
      mEventLoop->quit();
      return;
    }
  }

  mRequestTimer.restart();
}

#ifndef QT_NO_SSL
void HttpUploader::OnSslErrors(const QList<QSslError>& errors)
{
  Q_UNUSED(errors);

  if (mNetReply) {
    const QSslCertificate& cert = mNetReply->sslConfiguration().peerCertificate();
    QByteArray certPem = cert.toPem();
    certPem.replace("\r", "");
    certPem.replace("\n", "");
    LoadLocalCert();
    if (certPem != mServerCert) {
      LOG_ERROR_ONCE(QString("SSL untrusted certificate: %1").arg(cert.toPem().constData()));
      return;
    }
    Log.Trace(QString("Peer use trusted cert"));
    mNetReply->ignoreSslErrors();
  }
}
#endif


HttpUploader::HttpUploader()
  : mEventLoop(nullptr), mNetManager(nullptr), mNetReply(nullptr), mTimeoutTimer(nullptr), mCtrl(nullptr)
  , mError(0), mTimeout(false), mAbort(false)
{
}

HttpUploader::~HttpUploader()
{
  Reset();
}
