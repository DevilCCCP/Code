#include <QProcess>
#include <QTemporaryFile>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "Rsa.h"


bool Rsa::CreatePem(const QString& keyFilename, const QString& certFilename)
{
  QStringList args = QStringList()
      << "req" << "-newkey" << "rsa" << "-sha256" << "-new" << "-nodes"
      << "-keyout" << keyFilename << "-out" << certFilename << "-subj" << "/CN=localhost";
  int ret = QProcess::execute("openssl", args);
  return ret == 0;
}

bool Rsa::CreatePem(QByteArray& keyText, QByteArray& certText)
{
  QTemporaryFile key;
  QTemporaryFile cert;
  if (!key.open() || !cert.open()) {
    return false;
  }

  if (!CreatePem(key.fileName(), cert.fileName())) {
    return false;
  }
  keyText = key.readAll();
  certText = cert.readAll();
  return !keyText.isEmpty() && !certText.isEmpty();
}

RsaS Rsa::Create(int bits)
{
  BIGNUM* e = BN_new();
  int ret = BN_set_word(e, RSA_F4);
  if (ret != 1) {
    return RsaS();
  }

  rsa_stS rsaPtr(RSA_new(), RSA_free);
  ret = RSA_generate_key_ex(rsaPtr.data(), bits, e, nullptr);
  if (ret != 1) {
    return RsaS();
  }

  RsaS rsa(new Rsa());
  rsa->mRsa = rsaPtr;
  return rsa;
}

//RsaS Rsa::FromTlv(const TlvS& tlv, QString& errorText)
//{
//  int index = 1;
//  QByteArray modulus = tlv->Childs[index++]->Value;
//  QByteArray exponent = tlv->Childs[index++]->Value;
//  QByteArray d = tlv->Childs[index++]->Value;
//  QByteArray p = tlv->Childs[index++]->Value;
//  QByteArray q = tlv->Childs[index++]->Value;
//  QByteArray dp = tlv->Childs[index++]->Value;
//  QByteArray dq = tlv->Childs[index++]->Value;
//  QByteArray incerseQ = tlv->Childs[index++]->Value;
//}

RsaS Rsa::FromPem(const QByteArray& pemText, QString* errorText)
{
  bio_stS bio(BIO_new(BIO_s_mem()), BIO_free_all);
  if (BIO_write(bio.data(), pemText.constData(), pemText.size()) <= 0) {
    if (errorText) {
      *errorText = "RSA key write to BIO fail";
    }
    return RsaS();
  }
  RSA* rsaPtr = nullptr;
  if (!PEM_read_bio_RSAPrivateKey(bio.data(), &rsaPtr, nullptr, nullptr)) {
    if (errorText) {
      *errorText = "RSA key read from BIO fail";
    }
    return RsaS();
  }
  RsaS rsa(new Rsa());
  rsa->mRsa.reset(rsaPtr, RSA_free);
  return rsa;
}

RsaS Rsa::FromPemPub(const QByteArray& pemText, QString* errorText)
{
  bio_stS bio(BIO_new(BIO_s_mem()), BIO_free_all);
  if (BIO_write(bio.data(), pemText.constData(), pemText.size()) <= 0) {
    if (errorText) {
      *errorText = "RSA key write to BIO fail";
    }
    return RsaS();
  }
  RSA* rsaPtr = nullptr;
  if (!PEM_read_bio_RSAPublicKey(bio.data(), &rsaPtr, nullptr, nullptr)) {
    if (errorText) {
      *errorText = "RSA key read from BIO fail";
    }
    return RsaS();
  }
  RsaS rsa(new Rsa());
  rsa->mRsa.reset(rsaPtr, RSA_free);
  return rsa;
}

QByteArray Rsa::EncryptPub(const QByteArray& data)
{
  unsigned char buffer[4098];
  int size = RSA_public_encrypt(data.size(), (const unsigned char*)data.constData()
                                , buffer, mRsa.data(), RSA_PKCS1_PADDING);
  if (size > 0) {
    return QByteArray((const char*)buffer, size);
  } else {
    mErrorText = QString("Encrypt failed");
    return QByteArray();
  }
}

QByteArray Rsa::DecryptPriv(const QByteArray& data)
{
  unsigned char buffer[4098];
  int size = RSA_private_decrypt(data.size(), (const unsigned char*)data.constData()
                                 , buffer, mRsa.data(), RSA_PKCS1_PADDING);
  if (size > 0) {
    return QByteArray((const char*)buffer, size);
  } else {
    mErrorText = QString("Decrypt failed");
    return QByteArray();
  }
}

QByteArray Rsa::SignSha256(const QByteArray& data)
{
  if (!mRsa) {
    mErrorText = "RSA sign fail: PK not initialized";
    return QByteArray();
  }
  uchar hash[SHA256_DIGEST_LENGTH];
  if (!SHA256(reinterpret_cast<const uchar*>(data.constBegin()), data.size(), hash)) {
    mErrorText = "SHA256 calc fail";
    return QByteArray();
  }
  QByteArray sign;
  uint size = RSA_size(mRsa.data());
  sign.resize(size);
  if (RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, reinterpret_cast<uchar*>(sign.data()), &size, mRsa.data()) <= 0) {
    mErrorText = "RSA sign generate fail";
    return QByteArray();
  }
  return sign;
}

bool Rsa::VerifySha256(const QByteArray& data, const QByteArray& sign)
{
  if (!mRsa) {
    mErrorText = "RSA sign fail: PK not initialized";
    return false;
  }
  uchar hash[SHA256_DIGEST_LENGTH];
  if (!SHA256(reinterpret_cast<const uchar*>(data.constBegin()), data.size(), hash)) {
    mErrorText = "SHA256 calc fail";
    return false;
  }
  if (RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, reinterpret_cast<const uchar*>(sign.data()), (unsigned int)sign.size(), mRsa.data()) <= 0) {
    mErrorText = "RSA verify fail";
    return false;
  }
  return true;
}

QByteArray Rsa::PrivateToPem()
{
  bio_stS bio(BIO_new(BIO_s_mem()), BIO_free_all);
  PEM_write_bio_RSAPrivateKey(bio.data(), mRsa.data(), nullptr, nullptr, 0, nullptr, nullptr);

  QByteArray pemText;
  int pemlen = BIO_pending(bio.data());
  pemText.resize(pemlen);
  BIO_read(bio.data(), pemText.data(), pemText.size());
  return pemText;
}

QByteArray Rsa::PublicToPem()
{
  bio_stS bio(BIO_new(BIO_s_mem()), BIO_free_all);
  PEM_write_bio_RSAPublicKey(bio.data(), mRsa.data());

  QByteArray pemText;
  int pemlen = BIO_pending(bio.data());
  pemText.resize(pemlen);
  BIO_read(bio.data(), pemText.data(), pemText.size());
  return pemText;
}


Rsa::Rsa()
{
}
