#include <QtGlobal>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mPort(nullptr)
{
  ui->setupUi(this);

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
  ui->checkBoxBreakEn->setEnabled(false);
#endif
  ui->pushButtonClose->setVisible(false);
  InitSerials();
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent* event)
{
  if (mPort) {
    mPort->deleteLater();
  }

  QMainWindow::closeEvent(event);
}

void MainWindow::InitSerials()
{
  QStringList flowControls = QStringList() << "NoFlowControl" << "HardwareControl"
                                             << "SoftwareControl"/* << "UnknownFlowControl"*/;
  int index = 0;
  mFlowControl[QSerialPort::NoFlowControl] = index++;
  mFlowControl[QSerialPort::HardwareControl] = index++;
  mFlowControl[QSerialPort::SoftwareControl] = index++;
  mFlowControlValue.append(QSerialPort::NoFlowControl);
  mFlowControlValue.append(QSerialPort::HardwareControl);
  mFlowControlValue.append(QSerialPort::SoftwareControl);
//  mFlowControl[QSerialPort::UnknownFlowControl] = index++;
  ui->comboBoxFlowControl->addItems(flowControls);

  QStringList parities = QStringList() << "NoParity" << "EvenParity" << "OddParity"
                                     << "SpaceParity" << "MarkParity"/* << "UnknownParity"*/;
  index = 0;
  mParity[QSerialPort::NoParity] = index++;
  mParity[QSerialPort::EvenParity] = index++;
  mParity[QSerialPort::OddParity] = index++;
  mParity[QSerialPort::SpaceParity] = index++;
  mParity[QSerialPort::MarkParity] = index++;
  mParityValue.append(QSerialPort::NoParity);
  mParityValue.append(QSerialPort::EvenParity);
  mParityValue.append(QSerialPort::OddParity);
  mParityValue.append(QSerialPort::SpaceParity);
  mParityValue.append(QSerialPort::MarkParity);
//  mParity[QSerialPort::UnknownParity] = index++;
  ui->comboBoxParity->addItems(parities);

  QStringList stopBits = QStringList() << "OneStop" << "OneAndHalfStop" << "TwoStop"
                                       /*<< "UnknownStopBits"*/;
  index = 0;
  mStopBits[QSerialPort::OneStop] = index++;
  mStopBits[QSerialPort::OneAndHalfStop] = index++;
  mStopBits[QSerialPort::TwoStop] = index++;
  mStopBitsValue.append(QSerialPort::OneStop);
  mStopBitsValue.append(QSerialPort::OneAndHalfStop);
  mStopBitsValue.append(QSerialPort::TwoStop);
//  mStopBits[QSerialPort::UnknownStopBits] = index++;
  ui->comboBoxStopBits->addItems(stopBits);

  while (ui->comboBoxPorts->count() > 0) {
    ui->comboBoxPorts->removeItem(0);
  }

  mPortsList = QSerialPortInfo::availablePorts();
  foreach (const QSerialPortInfo& portInfo, mPortsList) {
    if (!portInfo.serialNumber().isEmpty()) {
      ui->comboBoxPorts->addItem(QString("COM %1 (%2)")
                                 .arg(portInfo.serialNumber()).arg(portInfo.portName()));
    } else {
      ui->comboBoxPorts->addItem(portInfo.portName());
    }
  }
  ui->widgetSerial->setEnabled(false);
}

void MainWindow::WriteLine()
{
  if (mLineWritten < mPrintImage.height()) {
    QByteArray data(mPrintImage.width()/8, (uchar)0);
    char* img = data.data();

    int iByte = 0;
    int iMask = 0x80;
    for (int i = 0; iByte < mPrintImage.width()/8; i++) {
      QColor color = QColor::fromRgb(mPrintImage.pixel(i, mLineWritten));
      if (color.value() < 0x80) {
        img[iByte] |= iMask;
      }
      iMask >>= 1;
      if (iMask == 0) {
        iMask = 0x80;
        iByte++;
      }
    }
    QByteArray fullcmd = data;
    mBytesWritten += fullcmd.size();
    mLineWritten++;
    mPort->write(fullcmd);
    mLog.append(QString("<p style=\"color:blue\">write: 0x%1</p>").arg(fullcmd.toHex().constData()));
    UpdateLog();
  } else {
    disconnect(mPort, &QSerialPort::bytesWritten, 0, 0);
  }
}

void MainWindow::UpdateLog()
{
  ui->textEditLog->setHtml(mLog);
  QTextCursor cursor = ui->textEditLog->textCursor();
  cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
  ui->textEditLog->setTextCursor(cursor);
}

void MainWindow::OnReadyRead()
{
  bool isAscii = true;
  QByteArray data = mPort->readAll();
  foreach (char ch, data) {
    if (ch < 0x20 || ch > 0x7f) {
      isAscii = false;
      break;
    }
  }
  if (data.size() > 20) {
    mLog.append(QString("<p>read %1 bytes</p>").arg(data.size()));
  } else if (isAscii) {
    mLog.append(QString("<p>read %1(0x%2)</p>").arg(data.constData()).arg(data.toHex().constData()));
  } else {
    mLog.append(QString("<p>read 0x%1</p>").arg(data.toHex().constData()));
  }
  UpdateLog();
}

void MainWindow::OnError(QSerialPort::SerialPortError serialPortError)
{
  Q_UNUSED(serialPortError);

  mLog.append(QString("<p style=\"color:red\">error: %1 (%2)</p>").arg(mPort->errorString()).arg((int)mPort->error()));
  UpdateLog();
}

void MainWindow::OnClose()
{
  mLog.append(QString("<p style=\"color:blue\">port closed</p>"));
  UpdateLog();
}

void MainWindow::OnBytesWritten(qint64 bytes)
{
  mBytesWritten -= bytes;
  if (!mBytesWritten) {
    WriteLine();
  }
}

void MainWindow::OnChange()
{
  ui->spinBoxBaudRateRead->setValue(mPort->baudRate(QSerialPort::Input));
  ui->spinBoxBaudRateWrite->setValue(mPort->baudRate(QSerialPort::Output));
  ui->spinBoxDataBits->setValue(mPort->dataBits());
  ui->comboBoxFlowControl->setCurrentIndex(mFlowControl[mPort->flowControl()]);
  ui->comboBoxParity->setCurrentIndex(mParity[mPort->parity()]);
  ui->comboBoxStopBits->setCurrentIndex(mStopBits[mPort->stopBits()]);
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
  ui->checkBoxBreakEn->setChecked(mPort->isBreakEnabled());
#endif
  ui->checkBoxDtr->setChecked(mPort->isDataTerminalReady());
  ui->checkBoxRts->setChecked(mPort->isRequestToSend());
}

void MainWindow::on_pushButtonSelect_clicked()
{
  int portIndex = ui->comboBoxPorts->currentIndex();
  if (portIndex < 0) {
    return;
  }

  mLog.clear();
  mPort = new QSerialPort(mPortsList.at(portIndex), this);
//  mPort->setReadBufferSize(20);
//  mPort->setBaudRate(QSerialPort::Baud4800, QSerialPort::Input);
//  mPort->setBaudRate(QSerialPort::Baud4800, QSerialPort::Output);
  if (!mPort->open(QSerialPort::ReadWrite)) {
    mLog.append(QString("<p style=\"color:red\">not connected to %1</p>").arg(mPort->portName()));
    UpdateLog();
    return;
  }
  OnChange();
  mLog.append(QString("<p style=\"color:green\">connected to %1</p>").arg(mPort->portName()));
  UpdateLog();
  ui->widgetSerial->setEnabled(true);
  mPort->clearError();

  connect(mPort, &QSerialPort::readyRead, this, &MainWindow::OnReadyRead);
  connect(mPort, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError serialPortError)>(&QSerialPort::error), this, &MainWindow::OnError);
  connect(mPort, &QSerialPort::aboutToClose, this, &MainWindow::OnClose);

  connect(mPort, &QSerialPort::baudRateChanged, this, &MainWindow::OnChange);
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
  connect(mPort, &QSerialPort::breakEnabledChanged, this, &MainWindow::OnChange);
#endif
  connect(mPort, &QSerialPort::dataBitsChanged, this, &MainWindow::OnChange);
  connect(mPort, &QSerialPort::dataTerminalReadyChanged, this, &MainWindow::OnChange);
  connect(mPort, &QSerialPort::flowControlChanged, this, &MainWindow::OnChange);
  connect(mPort, &QSerialPort::parityChanged, this, &MainWindow::OnChange);
  connect(mPort, &QSerialPort::requestToSendChanged, this, &MainWindow::OnChange);
  connect(mPort, &QSerialPort::stopBitsChanged, this, &MainWindow::OnChange);

  ui->pushButtonClose->setVisible(true);
  ui->pushButtonSelect->setVisible(false);
}

void MainWindow::on_pushButtonSendText_clicked()
{
  QString text = ui->lineEditText->text();
  QByteArray data = text.toLocal8Bit();
  if (!data.isEmpty()) {
    mPort->write(data);
    mLog.append(QString("<p style=\"color:blue\">write: %1(0x%2)</p>").arg(text).arg(data.toHex().constData()));
    UpdateLog();
  }
}

void MainWindow::on_pushButtonSendHex_clicked()
{
  QString text = ui->lineEditHex->text();
  QByteArray data = QByteArray::fromHex(text.toLatin1());
  if (!data.isEmpty()) {
    mPort->write(data);
    mLog.append(QString("<p style=\"color:blue\">write: 0x%1</p>").arg(data.toHex().constData()));
    UpdateLog();
  }
}

void MainWindow::on_pushButtonCustom_clicked()
{
  if (mPrintImage.isNull()) {
    mPrintImage = QImage("C:\\!Code\\!Code\\Telo\\Docs\\Test.png");
  }

  QByteArray data = QByteArray::fromHex(QByteArray("1d2a"));
  data.append((char)(mPrintImage.width()/8));
  data.append((char)(mPrintImage.height()/8));
  mPort->write(data);
  mBytesWritten += data.size();
  mLog.append(QString("<p style=\"color:blue\">write: 0x%1</p>").arg(data.toHex().constData()));
  UpdateLog();

  mLineWritten = 0;
  WriteLine();

  connect(mPort, &QSerialPort::bytesWritten, this, &MainWindow::OnBytesWritten);
}

void MainWindow::on_pushButtonClose_clicked()
{
  mPort->deleteLater();
  mPort = nullptr;

  ui->pushButtonClose->setVisible(false);
  ui->pushButtonSelect->setVisible(true);
  ui->widgetSerial->setEnabled(false);
}

void MainWindow::on_pushButtonSet_clicked()
{
#ifdef Q_OS_WIN32
  mPort->setBaudRate(ui->spinBoxBaudRateRead->value(), QSerialPort::AllDirections);
#else
  mPort->setBaudRate(ui->spinBoxBaudRateRead->value(), QSerialPort::Input);
  mPort->setBaudRate(ui->spinBoxBaudRateWrite->value(), QSerialPort::Output);
#endif
  mPort->setDataBits((QSerialPort::DataBits)ui->spinBoxDataBits->value());
  mPort->setFlowControl((QSerialPort::FlowControl)mFlowControlValue[ui->comboBoxFlowControl->currentIndex()]);
  mPort->setParity((QSerialPort::Parity)mParityValue[ui->comboBoxParity->currentIndex()]);
  mPort->setStopBits((QSerialPort::StopBits)mStopBitsValue[ui->comboBoxStopBits->currentIndex()]);
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
  mPort->setBreakEnabled(ui->checkBoxBreakEn->isChecked());
#endif
  mPort->setDataTerminalReady(ui->checkBoxDtr->isChecked());
  mPort->setRequestToSend(ui->checkBoxRts->isChecked());
}


void MainWindow::on_lineEditHex_returnPressed()
{
  on_pushButtonSendHex_clicked();
}

void MainWindow::on_lineEditText_returnPressed()
{
  on_pushButtonSendText_clicked();
}
