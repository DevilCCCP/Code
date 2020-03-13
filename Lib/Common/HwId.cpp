#include <QNetworkInterface>
#include <QThread>
#include <QFile>

#include "HwId.h"


QString HwId::Get()
{
#ifdef __arm__ // Raspberry proc uuid
  const char* kCpuidPath = "/proc/cpuinfo";
  const QByteArray kSerialLine("Serial          : ");
  QFile file(kCpuidPath);
  for (int j = 0; j < 100; j++) {
    if (file.open(QFile::ReadOnly)) {
      while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (line.startsWith(kSerialLine)) {
          QByteArray serial = line.mid(kSerialLine.size());
          return QString::fromLatin1(serial);
        }
      }
    }
    QThread::msleep(1);
  }
#endif
  foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
    if (!interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
      return interface.hardwareAddress();
    }
  }
  return QString();
}
