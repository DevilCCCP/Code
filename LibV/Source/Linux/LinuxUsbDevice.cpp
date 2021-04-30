#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QRegExp>

#include "LinuxUsbDevice.h"


QString GetUsbPath(const QString& name)
{
  // -------------------------------------------------
  // pci-0000:00:14.0-usb-0:1:1.0-video-index0
  // .......................^.................
  // platform-sunxi-ehci.4-usb-0:1:1.0-video-index0
  // ....................^.........................
  // platform-3f980000.usb-usb-0:1.4:1.0-video-index0
  // ..............................^.................
  QRegExp patternUsb(".*-usb-\\d+:([\\d\\.]+):.*");
  if (patternUsb.exactMatch(name)) {
    return patternUsb.cap(1);
  }
  return QString();
}

QString LinuxUsbDevice(const QString& usbHubPath)
{
  if (usbHubPath == "-") {
    return LinuxNotUsbDevice();
  }

  QDir v4l("/dev/v4l/by-path");
  QRegExp usbHubPathPattern(usbHubPath, Qt::CaseSensitive, QRegExp::Wildcard);
  auto links = v4l.entryInfoList(QDir::Files);
  for (auto itr = links.begin(); itr != links.end(); itr++) {
    const QFileInfo& info = *itr;
    QString usbPath = GetUsbPath(info.fileName());
    if (!usbPath.isEmpty() && usbHubPathPattern.exactMatch(usbPath)) {
      return info.canonicalFilePath();
    }
  }
  return QString();
}

QString LinuxNotUsbDevice()
{
  QList<QString> usbDevices;
  QDir v4l("/dev/v4l/by-path");
  auto links = v4l.entryInfoList(QDir::Files);
  for (auto itr = links.begin(); itr != links.end(); itr++) {
    const QFileInfo& info = *itr;
    QString usbPath = GetUsbPath(info.fileName());
    if (!usbPath.isEmpty()) {
      usbDevices << info.canonicalFilePath();
    }
  }

  int maxDev = 8;
  for (int i = 0; i < maxDev; i++) {
    QString videoDev = QString("/dev/video%1").arg(i);
    if (QFile::exists(videoDev)) {
      if (!usbDevices.contains(videoDev)) {
        return videoDev;
      }
      maxDev = i + 8;
    }
  }
  return QString();
}
