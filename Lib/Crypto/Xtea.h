#pragma once

#include <QVector>
#include <QByteArray>


class Xtea
{
  QVector<quint32> mKey;

public:
  void SetKey(const QVector<quint32>& _Key) { mKey = _Key; }
  bool IsValid() const { return !mKey.isEmpty(); }

public:
  void SetKeyData(const QByteArray& keyData);
  void DoubleEncrypt(const QByteArray& data, QByteArray& encryptedData);
  void DoubleDecrypt(const QByteArray& encryptedData, QByteArray& data);

private:
  void EncryptOne(const quint32* data, quint32* encryptedData, const quint32* key);
  void DecryptOne(const quint32* data, quint32* decryptedData, const quint32* key);

public:
  Xtea();
  Xtea(const QByteArray& keyData);
};
