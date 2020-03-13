#include <QDir>
#include <QFileInfo>
#include <QDirIterator>

#include "LinuxUsbDevice.h"


int GetUsbNumber(const QString& name)
{
  // platform-sunxi-ehci.4-usb-0:1:1.0-video-index0
  // ....................^.........................
  // platform-3f980000.usb-usb-0:1.4:1.0-video-index0
  // ..............................^.................
  // not supported:
  // pci-0000:00:14.0-usb-0:1:1.0-video-index0
  // .......................^.................
  QStringList parts = name.split('.');
  bool hasUsb = false;
  foreach (const QString& part, parts) {
    bool hasUsbPart = false;
    int number = -1;
    QString subString;
    QString subNumber;
    for (int i = 0; i < part.size(); i++) {
      QChar ch = part.at(i);
      if (ch.isLetter()) {
        subString.append(ch);
      } else if (ch.isNumber()) {
        subNumber.append(ch);
      } else {
        if (!subNumber.isEmpty()) {
          number = subNumber.toInt();
          if (hasUsb) {
            return number; // '.usb-usb-0:1.4' variant
          }
          subNumber.clear();
        }
        if (subString == "usb") {
          hasUsbPart = true;
          if (number >= 0) { // '.4-usb' variant
            return number;
          }
        }
        subString.clear();
      }
    }
    if (hasUsbPart) {
      hasUsb = true;
    }
  }
  return -1;
}

QString LinuxUsbDevice(int number)
{
  if (number < 0) {
    return LinuxNotUsbDevice();
  }

  QDir v4l("/dev/v4l/by-path");
  auto links = v4l.entryInfoList(QDir::Files);
  for (auto itr = links.begin(); itr != links.end(); itr++) {
    const QFileInfo& info = *itr;
    int usbNumber = GetUsbNumber(info.fileName());
    if (usbNumber == number) {
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
    int usbNumber = GetUsbNumber(info.fileName());
    if (usbNumber >= 0) {
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
