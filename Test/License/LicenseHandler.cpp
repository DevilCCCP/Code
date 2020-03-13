#include <QCoreApplication>
#include <QCryptographicHash>

#include <Lib/Include/License.h>
#include <Lib/Log/Log.h>

#include "LicenseHandler.h"


bool LicenseHandler::Post(const QString& path, const QList<QByteArray>& params, const QList<File>& files)
{
  Q_UNUSED(params);

  if (path == "/lilo" && !files.isEmpty()) {
    HttpResultOk(true);

    QByteArray sn = files.first().Data;
    Log.Info(QString("S/n: %1").arg(QString::fromUtf8(sn)));

    for (int i = 0; i <= kLcCount; i++) {
      QCryptographicHash hash(QCryptographicHash::Md5);
      hash.addData(sn);
      if (i) {
        hash.addData((char*)&i, 4);
      }
      Log.Info(QString("Line %1: %2").arg(i, 2).arg(QString(hash.result().toHex())));
      if (i) {
        Answer().append(hash.result().toHex().prepend(':'));
      } else {
        Answer().append(hash.result().toHex());
      }
    }
    return true;
  }

  HttpResultFail(true);
  return true;
}


LicenseHandler::LicenseHandler()
{
}

