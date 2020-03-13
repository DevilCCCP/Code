#include <QSettings>
#include <QElapsedTimer>
#include <QSettings>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Crypto/Xtea.h>
#include <Lib/Log/Log.h>


const QByteArray kKey = QByteArray::fromHex("0A90F7D11B6BEC4B018C8F6A50849F51C4D4CEDD8DBA81D6CC1B22A19F2416DB");
const QByteArray kDataDecrypted = QByteArray::fromHex("01020304050607091112131415161718");
const QByteArray kDataEncrypted = QByteArray::fromHex("6726CD2D5DEF91DAD40B31794559B478");

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  Xtea xtea(kKey);
  QByteArray result;
  Log.Info("Test encrypt 8");
  xtea.DoubleEncrypt(kDataDecrypted.left(8), result);
  Log.Info(QString("Resuld: %1").arg(result.toHex().constData()));
  if (result != kDataEncrypted.left(8)) {
    Log.Error("Fail");
    return 1;
  }
  Log.Info("Test encrypt 16");
  xtea.DoubleEncrypt(kDataDecrypted, result);
  Log.Info(QString("Resuld: %1").arg(result.toHex().constData()));
  if (result != kDataEncrypted) {
    Log.Error("Fail");
    return 1;
  }

  Log.Info("Test decrypt 8");
  xtea.DoubleDecrypt(kDataEncrypted.left(8), result);
  Log.Info(QString("Resuld: %1").arg(result.toHex().constData()));
  if (result != kDataDecrypted.left(8)) {
    Log.Error("Fail");
    return 1;
  }
  Log.Info("Test decrypt 16");
  xtea.DoubleDecrypt(kDataEncrypted, result);
  Log.Info(QString("Resuld: %1").arg(result.toHex().constData()));
  if (result != kDataDecrypted) {
    Log.Error("Fail");
    return 1;
  }

  Log.Info("Success");
  return 0;
}

