#include <QCoreApplication>

#include <QThread>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>


int main(int argc, char *argv[])
{
  QCoreApplication b(argc, argv);

  if (argc < 2) {
    qDebug() << "Usage: exe com_path";
    auto list = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo portInfo, list) {
      qDebug() << portInfo.portName();
    }
    return 0;
  }

  QSerialPort printer(argv[1]);
  if (!printer.open(QSerialPort::ReadWrite)) {
    qDebug() << "Open fail";
    return -1;
  }

  int counter = 0;
  forever {
    QByteArray writeData = QByteArray::fromHex(QByteArray("100401"));
    qDebug() << writeData.toHex() << " (" << ++counter << ")";
    printer.write(writeData);
    if (!printer.waitForBytesWritten(800)) {
      qDebug() << "write fail";
      return -1;
    }
    if (!printer.waitForReadyRead(800)) {
      qDebug() << "read fail";
      qDebug() << "error: " << printer.errorString() << " == " << printer.error();
      qDebug() << "pinoutSignals: " << QString::number((int)printer.pinoutSignals(), 16);
    }
    QByteArray readData = printer.readAll();
    qDebug() << readData.toHex();
    QThread::msleep(2);
  }

  return 0;
}
