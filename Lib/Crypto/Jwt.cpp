#include <QDateTime>

#include "Jwt.h"
#include "Rsa.h"
#include "Tools.h"


QByteArray Jwt::Create(const QByteArray& email, const QByteArray& scope, RsaS& privateKey, QString& errorText)
{
  QByteArray head = "{\"alg\":\"RS256\",\"typ\":\"JWT\"}";
  qint64 secs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;
  QByteArray claim = QByteArray("{")
      + "\"iss\":\"" + email + "\","
      + "\"sub\":\"lenin200615@gmail.com\","
      + "\"scope\":\"" + scope + "\","
      + "\"aud\":\"https://www.googleapis.com/oauth2/v3/token\","
      + "\"exp\":" + QByteArray::number(secs + 3600) + ","
      + "\"iat\":" + QByteArray::number(secs)
      + "}";
  return CreateRaw(head, claim, privateKey, errorText);
}

QByteArray Jwt::CreateRaw(const QByteArray& header, const QByteArray& payload, RsaS& privateKey, QString& errorText)
{
  QByteArray jwsText = GetUrlEncodedBase64(header) + "." + GetUrlEncodedBase64(payload);

  QByteArray sign = privateKey->SignSha256(jwsText);
  if (sign.isEmpty()) {
    errorText = privateKey->ErrorText();
    return QByteArray();
  }

  return jwsText + "." + GetUrlEncodedBase64(sign);
}
