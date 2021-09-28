#pragma once

#include <QByteArray>

#include <Lib/Include/Common.h>


DefineClassS(Rsa);

class Jwt
{
  QString mErrorText;

public:
  const QString& ErrorText() const { return mErrorText; }

public:
  static QByteArray Create(const QByteArray& email, const QByteArray& scope, RsaS& privateKey, int expireSecs, QString* errorText = nullptr);
  static QByteArray CreateRaw(const QByteArray& header, const QByteArray& payload, RsaS& privateKey, QString* errorText = nullptr);
};
