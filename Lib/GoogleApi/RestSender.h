#pragma once

#include <QObject>
#include <QElapsedTimer>

#include <Lib/Include/Common.h>


class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QEventLoop;
class QTimer;

class RestSender: public QObject
{
  PROPERTY_GET_SET(bool,  Debug)

  QNetworkAccessManager*  mNetManager;
  QEventLoop*             mEventLoop;
  QTimer*                 mNetAliveTimer;
  QElapsedTimer           mSendTimer;
  int                     mSendTimeoutMs;

  QString                 mUri;
  QString                 mToken;
  QByteArray              mRespondData;

  Q_OBJECT

public:
  const QByteArray& RespondData() const { return mRespondData; }

public:
  void SetBearer(const QString& token);
  bool SendForm(const QString& uri, const QByteArray& data);
  bool SendJson(const QString& uri, const QByteArray& jsonData);
  bool SendJson2(const QString& uri, const QByteArray& jsonData);
  bool SendFile(const QString& uri, const QByteArray& fileData);
  bool SendRequest(const QString& uri);

private:
  void Prepare();
  bool PostRequest(QNetworkRequest& request, const QByteArray& data);
  bool PutRequest(QNetworkRequest& request, const QByteArray& data);
  bool GetRequest(QNetworkRequest& request);
  bool ExecRequest(QNetworkReply* netReply);

private slots:
  void OnAliveTimeout();

public slots:
  void Abort();

signals:
  void SayAlive();

public:
  RestSender(QObject* parent = nullptr);
};
