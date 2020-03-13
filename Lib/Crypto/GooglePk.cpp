#include <Lib/Include/Tlv.h>

#include "GooglePk.h"
#include "Rsa.h"


const QByteArray kBeginPk("-----BEGIN PRIVATE KEY-----");
const QByteArray kEndPk("-----END PRIVATE KEY-----");

RsaS GooglePk::FromText(const QByteArray& pkText, QString& errorText)
{
  int indBegin = pkText.indexOf(kBeginPk);
  int indEnd = pkText.indexOf(kEndPk);
  if (indBegin < 0 || indEnd < 0 || indBegin > indEnd) {
    errorText = QString("Private key format doesn't recognized (pk: '%1')").arg(QString::fromUtf8(pkText));
    return RsaS();
  }
  indBegin += kBeginPk.size();
  QByteArray pkRaw = pkText.mid(indBegin, indEnd - indBegin);
  QByteArray pkTlv = QByteArray::fromBase64(pkRaw);
  TlvParser parser(QList<int>() << 0x04 << 0x30);
  TlvS tlvRoot = parser.Parse(pkTlv);
  if (!tlvRoot) {
    errorText = QString("TLV parse fail (data: '%1'").arg(pkTlv.toHex().constData());
  }
  TlvS tlvKey = tlvRoot->At(0x30, 0x04, 0x30);
  tlvKey->Dump();
  const QByteArray& pkData = tlvRoot->Value;

  QByteArray pkBio("-----BEGIN RSA PRIVATE KEY-----\n");
  for (int i = 0; i < pkData.size(); i += 48) {
    QByteArray part = pkData.mid(i, 48);
    pkBio.append(part.toBase64() + '\n');
  }
  pkBio.append(QByteArray("-----END RSA PRIVATE KEY-----"));

  return Rsa::FromBio(pkBio, errorText);
}


GooglePk::GooglePk()
{
}

