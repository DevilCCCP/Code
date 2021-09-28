#include <QProcess>
#include <QTemporaryFile>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Aes.h"


const int kBlockSize = 16;

bool Aes::IsKeySizeValid(int size)
{
  // Размер ключа	128/192/256 бит (16/24/32 байта)
  return size == 16 || size == 24 || size == 32;
}

AesS Aes::Create(const QByteArray& key)
{
  if (!IsKeySizeValid(key.size())) {
    return AesS();
  }

  aes_key_stS keyStruct(new aes_key_st());
  int ret = AES_set_encrypt_key((const uchar*)key.constData(), key.size() * 8, keyStruct.data());
  if (ret != 0) {
    return AesS();
  }

  AesS aes(new Aes());
  aes->mAesKey = keyStruct;
  return aes;
}

QByteArray Aes::Encrypt(const QByteArray& data)
{
  if (data.size() % kBlockSize != 0) {
    mErrorText = QString("AES Encrypt failed, bad data size");
    return QByteArray();
  }
  mErrorText.clear();

  int count = data.size() / kBlockSize;
  QByteArray result;
  result.resize(data.size());
  for (int i = 0; i < count; i++) {
    AES_encrypt((const uchar*)data.data() + kBlockSize * i, (uchar*)result.data() + kBlockSize * i, mAesKey.data());
  }
  return result;
}

QByteArray Aes::Decrypt(const QByteArray& data)
{
  if (data.size() % kBlockSize != 0) {
    mErrorText = QString("AES Decrypt failed, bad data size");
    return QByteArray();
  }
  mErrorText.clear();

  int count = data.size() / kBlockSize;
  QByteArray result;
  result.resize(data.size());
  for (int i = 0; i < count; i++) {
    AES_decrypt((const uchar*)data.data() + kBlockSize * i, (uchar*)result.data() + kBlockSize * i, mAesKey.data());
  }
  return result;
}


Aes::Aes()
{
}
