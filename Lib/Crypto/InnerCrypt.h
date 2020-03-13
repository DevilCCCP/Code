#pragma once

#include <QByteArray>

#include <Lib/Include/Common.h>


DefineClassS(InnerCrypt);
DefineClassS(Rsa);

class InnerCrypt
{
  RsaS    mRsa;
  QString mErrorText;

public:
  const QString& ErrorText();

public:
  QByteArray Encrypt(const QByteArray& decData);
  QByteArray Decrypt(const QByteArray& encData);
  QByteArray Sign(const QByteArray& data);
  bool Verify(const QByteArray& data, const QByteArray& sign);

public:
  InnerCrypt();
};
