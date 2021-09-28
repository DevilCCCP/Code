#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

#include <Lib/Log/Log.h>

#include "RestSender.h"


const int kSendTimeoutDefaultMs = 30 * 1000;
const int kWorkPeriodMs = 500;

void RestSender::SetBearer(const QString& token)
{
  mToken = token;
}

bool RestSender::SendForm(const QString& uri, const QByteArray& data)
{
  Prepare();

  mUri = uri;

  QNetworkRequest netRequest = QNetworkRequest(QUrl(mUri));
  netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  netRequest.setHeader(QNetworkRequest::ContentLengthHeader, data.size());

  return PostRequest(netRequest, data);
}

bool RestSender::SendJson(const QString& uri, const QByteArray& jsonData)
{
  Prepare();

  mUri = uri;
  QNetworkRequest netRequest = QNetworkRequest(QUrl(mUri));
  netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
  netRequest.setHeader(QNetworkRequest::ContentLengthHeader, jsonData.size());

  return PostRequest(netRequest, jsonData);
}

bool RestSender::SendJson2(const QString& uri, const QByteArray& jsonData)
{
  Prepare();

  mUri = uri;
  QNetworkRequest netRequest = QNetworkRequest(QUrl(mUri));
  netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");
  netRequest.setHeader(QNetworkRequest::ContentLengthHeader, jsonData.size());

  return PutRequest(netRequest, jsonData);
}

bool RestSender::SendFile(const QString& uri, const QByteArray& fileData)
{
  Prepare();

  mUri = uri;
  QNetworkRequest netRequest = QNetworkRequest(QUrl(mUri));
  netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
  netRequest.setHeader(QNetworkRequest::ContentLengthHeader, fileData.size());

  return PostRequest(netRequest, fileData);
}

bool RestSender::SendRequest(const QString& uri)
{
  Prepare();

  mUri = uri;
  QNetworkRequest netRequest = QNetworkRequest(QUrl(mUri));

  return GetRequest(netRequest);
}

void RestSender::Prepare()
{
  mRespondData.clear();

  if (mNetManager) {
    return;
  }
  mNetManager    = new QNetworkAccessManager(this);
  mEventLoop     = new QEventLoop(this);
  mNetAliveTimer = new QTimer();

  connect(mNetAliveTimer, &QTimer::timeout, this, &RestSender::OnAliveTimeout);
}

bool RestSender::PostRequest(QNetworkRequest& request, const QByteArray& data)
{
  if (!mToken.isEmpty()) {
    request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
  }

  if (mDebug) {
    Log.Info(QString("=== POST request: ======\n") + mUri + "\n" + data.constData());
  }

  QNetworkReply* netReply = mNetManager->post(request, data);

  return ExecRequest(netReply);
}

bool RestSender::PutRequest(QNetworkRequest& request, const QByteArray& data)
{
  if (!mToken.isEmpty()) {
    request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
  }

  if (mDebug) {
    Log.Info(QString("=== PUT request: ======\n") + mUri + "\n" + data.constData());
  }

  QNetworkReply* netReply = mNetManager->put(request, data);

  return ExecRequest(netReply);
}

bool RestSender::GetRequest(QNetworkRequest& request)
{
  if (!mToken.isEmpty()) {
    request.setRawHeader("Authorization", "Bearer " + mToken.toLatin1());
  }

  if (mDebug) {
    Log.Info(QString("=== GET request: ======\n") + mUri);
  }

  QNetworkReply* netReply = mNetManager->get(request);

  return ExecRequest(netReply);
}

bool RestSender::ExecRequest(QNetworkReply* netReply)
{
  mNetAliveTimer->start(kWorkPeriodMs);

  QObject::connect(netReply, &QNetworkReply::finished, mEventLoop, &QEventLoop::quit);

  mEventLoop->exec();

  int code = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  mRespondData = netReply->readAll();
  if (mDebug && code != 0) {
    if (mDebug) {
      Log.Info(QString("=== Respond: ======\n") + mRespondData.constData());
    }
  }
  return code == 200;
}

void RestSender::OnAliveTimeout()
{
  if (mSendTimer.elapsed() > mSendTimeoutMs) {
    Abort();
  }

  emit SayAlive();
}

void RestSender::Abort()
{
  if (mEventLoop) {
    mEventLoop->quit();
  }
}

RestSender::RestSender(QObject* parent)
  : QObject(parent)
  , mDebug(false)
  , mNetManager(nullptr), mEventLoop(nullptr)
{
}
