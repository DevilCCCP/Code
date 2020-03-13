#pragma once

#include <QByteArray>


QByteArray GetUrlEncodedBase64(const QByteArray &data)
{
  int padding = data.size() % 3;
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  QByteArray dataBase64 = data.toBase64(QByteArray::Base64UrlEncoding);
#else
  QByteArray dataBase64 = data.toBase64();
#endif
  if (padding) {
    if (padding == 1) {
      dataBase64 = dataBase64.left(dataBase64.size() - 2);
    } else {
      dataBase64 = dataBase64.left(dataBase64.size() - 1);
    }
  }
  return dataBase64;
}
