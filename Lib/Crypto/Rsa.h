#pragma once

#include <QByteArray>

#include <Lib/Common/Tlv.h>


DefineClassS(Rsa);
DefineStructS(rsa_st);
DefineStructS(bio_st);

class Rsa
{
  rsa_stS mRsa;
  QString mErrorText;

public:
  const QString& ErrorText() const { return mErrorText; }

public:
  static bool CreatePem(const QString& keyFilename, const QString& certFilename);
  static bool CreatePem(QByteArray& keyText, QByteArray& certText);
  static RsaS Create(int bits);
  static RsaS FromPem(const QByteArray& pemText, QString* errorText = nullptr);
  static RsaS FromPemPub(const QByteArray& pemText, QString* errorText = nullptr);

  QByteArray EncryptPub(const QByteArray& data);
  QByteArray DecryptPriv(const QByteArray& data);
  QByteArray SignSha256(const QByteArray& data);
  bool VerifySha256(const QByteArray& data, const QByteArray& sign);

  QByteArray PrivateToPem();
  QByteArray PublicToPem();

private:
  Rsa();
};
