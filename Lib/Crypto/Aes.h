#pragma once

#include <QByteArray>

#include <Lib/Include/Common.h>

DefineClassS(Aes);
DefineStructS(aes_key_st);

class Aes
{
  aes_key_stS mAesKey;
  QString     mErrorText;

public:
  const QString& ErrorText() const { return mErrorText; }

public:
  static bool IsKeySizeValid(int size);
  static AesS Create(const QByteArray& key);

  QByteArray Encrypt(const QByteArray& data);
  QByteArray Decrypt(const QByteArray& data);

private:
  Aes();
};
