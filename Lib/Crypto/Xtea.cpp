#include <QtEndian>

#include <Lib/Log/Log.h>

#include "Xtea.h"


const int kKeySize = 4; // in quint32 units
const int kDoubleKeySize = 8; // in quint32 units
const int kDataBlockSize = sizeof(quint32) * 2; // in bytes

const int kXteaIterations = 64;
const quint32 kXteaDelta = 0x9E3779B9;

void Xtea::SetKeyData(const QByteArray& keyData)
{
  for (int i = 0; i + (int)sizeof(quint32) <= keyData.size(); i += (int)sizeof(quint32)) {
    mKey.append(qFromLittleEndian<quint32>((const uchar*)keyData.constData() + i));
  }
}

void Xtea::DoubleEncrypt(const QByteArray& data, QByteArray& encryptedData)
{
  encryptedData.clear();
  if (mKey.size() != 2*kKeySize) {
    Log.Error(QString("Xtea double encrypt wrong key size (key: %1, expect: %2)").arg(mKey.size()).arg(2*kKeySize));
    return;
  }
  if (encryptedData.size() % kDataBlockSize != 0) {
    Log.Error(QString("Xtea double encrypt wrong data size (data: %1, expect: N*%2)").arg(data.size()).arg(kDataBlockSize));
    return;
  }
  int dataSize = data.size();
  const char* dataSrc = data.constData();
  encryptedData.resize(dataSize);
  char* dataDst = encryptedData.data();
  while (dataSize > 0) {
    quint32 unitsSrc[2];
    quint32 unitsTmp[2];
    quint32 unitsDst[2];
    unitsSrc[0] = qFromLittleEndian<quint32>((const uchar*)dataSrc);
    unitsSrc[1] = qFromLittleEndian<quint32>((const uchar*)dataSrc + sizeof(quint32));
    EncryptOne(unitsSrc, unitsTmp, mKey.constData());
    EncryptOne(unitsTmp, unitsDst, mKey.constData() + kKeySize);
    qToLittleEndian<quint32>(unitsDst[0], (uchar*)dataDst);
    qToLittleEndian<quint32>(unitsDst[1], (uchar*)dataDst + sizeof(quint32));
    dataSize -= kDataBlockSize;
    dataSrc  += kDataBlockSize;
    dataDst  += kDataBlockSize;
  }
}

void Xtea::DoubleDecrypt(const QByteArray& encryptedData, QByteArray& data)
{
  data.clear();
  if (mKey.size() != 2*kKeySize) {
    Log.Error(QString("Xtea double decrypt wrong key size (key: %1, expect: %2)").arg(mKey.size()).arg(2*kKeySize));
    return;
  }
  if (encryptedData.size() % kDataBlockSize != 0) {
    Log.Error(QString("Xtea double decrypt wrong data size (data: %1, expect: N*%2)").arg(data.size()).arg(kDataBlockSize));
    return;
  }
  int dataSize = encryptedData.size();
  const char* dataSrc = encryptedData.constData();
  data.resize(dataSize);
  char* dataDst = data.data();
  while (dataSize > 0) {
    quint32 unitsSrc[2];
    quint32 unitsTmp[2];
    quint32 unitsDst[2];
    unitsSrc[0] = qFromLittleEndian<quint32>((const uchar*)dataSrc);
    unitsSrc[1] = qFromLittleEndian<quint32>((const uchar*)dataSrc + sizeof(quint32));
    DecryptOne(unitsSrc, unitsTmp, mKey.constData() + kKeySize);
    DecryptOne(unitsTmp, unitsDst, mKey.constData());
    qToLittleEndian<quint32>(unitsDst[0], (uchar*)dataDst);
    qToLittleEndian<quint32>(unitsDst[1], (uchar*)dataDst + sizeof(quint32));
    dataSize -= kDataBlockSize;
    dataSrc  += kDataBlockSize;
    dataDst  += kDataBlockSize;
  }
}

void Xtea::EncryptOne(const quint32* data, quint32* encryptedData, const quint32* key)
{
  const quint32& x1 = data[0];
  const quint32& x2 = data[1];
  quint32&       y1 = encryptedData[0];
  quint32&       y2 = encryptedData[1];

  y1 = x1;
  y2 = x2;
  quint32 sum = 0;
  for (int i = 0; i < kXteaIterations; i++) {
    y1 += ((y2<<4 ^ y2>>5) + y2) ^ (sum + key[sum&0x03]);
    sum += kXteaDelta;
    y2 += ((y1<<4 ^ y1>>5) + y1) ^ (sum + key[(sum>>11)&0x03]);
  }
}

void Xtea::DecryptOne(const quint32* data, quint32* decryptedData, const quint32* key)
{
  const quint32& x1 = data[0];
  const quint32& x2 = data[1];
  quint32&       y1 = decryptedData[0];
  quint32&       y2 = decryptedData[1];

  y1 = x1;
  y2 = x2;
  quint32 sum = (quint32)kXteaDelta * (quint32)kXteaIterations;
//  for (int i = 0; i < kXteaIterations; i++) {
  while (sum != 0) {
    y2 -= (((y1<<4) ^ (y1>>5)) + y1) ^ (sum + key[(sum>>11)&0x03]);
    sum -= kXteaDelta;
    y1 -= (((y2<<4) ^ (y2>>5)) + y2) ^ (sum + key[sum&0x03]);
  }
}


Xtea::Xtea()
{

}

Xtea::Xtea(const QByteArray& keyData)
{
  SetKeyData(keyData);
}
