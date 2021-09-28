#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

#include <Lib/Crypto/Rsa.h>
#include <Lib/Crypto/Jwt.h>
#include <Lib/Log/Log.h>

#include "ServiceAccounts.h"
#include "RestSender.h"


const char* kDriveScope = "https://www.googleapis.com/auth/drive";
const char* kDriveSheetScope = "https://www.googleapis.com/auth/spreadsheets";
const int kTokenExpireSecs = 60 * 60;

bool ServiceAccount::IsValid(GoogleApi::Scope scope) const
{
  return !mToken.isEmpty() && mScopes.testFlag(scope);
}

bool ServiceAccount::IsValid(GoogleApi::Scopes scopes) const
{
  return !mToken.isEmpty() && (mScopes & scopes) == scopes;
}

const QString& ServiceAccount::Token() const
{
  return mToken;
}

void ServiceAccount::SetRestSender(RestSender* _RestSender)
{
  mRestSender = _RestSender;
}

void ServiceAccount::SetClientEmail(const QByteArray& clientEmail)
{
  mClientEmail = clientEmail;
}

bool ServiceAccount::LoadPrivateKeyPem(const QByteArray& privateKeyPem)
{
  QString errRsa;
  mPrivateKey = Rsa::FromPem(privateKeyPem, &errRsa);
  if (!mPrivateKey) {
    Log.Error(QString("Load private key failed (err: '%1')").arg(errRsa));
    return false;
  }
  return true;
}

bool ServiceAccount::LoadFromJson(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QFile::ReadOnly)) {
    Log.Error(QString("Open json file fail (err: '%1')").arg(file.errorString()));
    return false;
  }

  QByteArray data = file.readAll();
  if (data.isEmpty()) {
    Log.Error(QString("Read json file fail (err: '%1')").arg(file.errorString()));
    return false;
  }
  file.close();

  return LoadFromJson(data);
}

bool ServiceAccount::LoadFromJson(const QByteArray& jsonData)
{
  QJsonParseError errJson;
  QJsonDocument doc = QJsonDocument::fromJson(jsonData, &errJson);
  if (errJson.error != QJsonParseError::NoError) {
    Log.Error(QString("Parse json fail (err: '%1')").arg(errJson.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  mClientEmail = root.value("client_email").toString().toLatin1();
  if (mClientEmail.isEmpty()) {
    Log.Error(QString("Load client email failed"));
    return false;
  }
  QByteArray keyPem = root.value("private_key").toString().toLatin1();
  if (!LoadPrivateKeyPem(keyPem)) {
    return false;
  }
  return true;
}

bool ServiceAccount::Acquire(GoogleApi::Scope scope)
{
  mScopes.setFlag(scope);
  return Acquire();
}

bool ServiceAccount::Acquire(GoogleApi::Scopes scopes)
{
  mScopes = scopes;
  return Acquire();
}

bool ServiceAccount::Acquire()
{
  if (!mRestSender) {
    Log.Error(QString("Acquire without REST sender"));
    return false;
  }

  if (mClientEmail.isEmpty() || !mPrivateKey) {
    Log.Error(QString("Acquire without settings"));
    return false;
  }

  QString errText;
  QByteArray jwtData = Jwt::Create(mClientEmail, kDriveScope, mPrivateKey, kTokenExpireSecs, &errText);
  if (jwtData.isEmpty()) {
    Log.Error(QString("Create jwt fail (err: '%1')").arg(errText));
    return false;
  }

  QByteArray body = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion=" + jwtData;
  if (!mRestSender->SendForm("https://www.googleapis.com/oauth2/v3/token", body)) {
    return false;
  }

  const QByteArray& respondData = mRestSender->RespondData();
  if (respondData.isEmpty()) {
    Log.Error(QString("Authentication fail, empty answer"));
    return false;
  }

  QJsonParseError errJson;
  QJsonDocument doc = QJsonDocument::fromJson(respondData, &errJson);
  if (errJson.error != QJsonParseError::NoError) {
    Log.Error(QString("Authentication fail, parse respond json fail (err: '%1')").arg(errJson.errorString()));
    return false;
  }

  QJsonObject root = doc.object();
  mToken = root.value("access_token").toString();
  QString tokenType = root.value("token_type").toString();
  if (mToken.isEmpty()) {
    Log.Error(QString("Authentication fail, empty token"));
    return false;
  }
  if (tokenType != "Bearer") {
    Log.Error(QString("Authentication fail, bad token type"));
    return false;
  }
  return true;
}

ServiceAccount::ServiceAccount()
  : mRestSender(nullptr)
{
}
