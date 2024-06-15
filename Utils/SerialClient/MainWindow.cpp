#include <QtGlobal>

#include "MainWindow.h"
#include "ui_MainWindow.h"


int kReadTimeoutMs = 500;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mReadTimer(new QTimer(this))
  , mPort(nullptr)
{
  ui->setupUi(this);

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
  ui->checkBoxBreakEn->setEnabled(false);
#endif
  ui->pushButtonClose->setVisible(false);
  InitSerials();

  ui->pushButtonCustom->setVisible(false);

  mReadTimer->setSingleShot(true);

  connect(mReadTimer, &QTimer::timeout, this, &MainWindow::OnReadTimeout);
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
      ui->comboBoxPorts->addItem(QString("COM %1 (%2, %3)")
                                 .arg(portInfo.serialNumber(), portInfo.portName(), portInfo.description()));
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
    mLog.append(QString("<p style=\"color:blue\">0x%1</p>").arg(fullcmd.toHex().constData()));
    UpdateLog();
  } else {
    disconnect(mPort, &QSerialPort::bytesWritten, 0, 0);
  }
}

void MainWindow::ShowData(bool force)
{
  bool isAscii = true;
  int iLineStart = 0;
  bool needUpdate = false;
  for (int i = 0; i < mReadBuffer.size(); i++) {
    uchar b = (uchar)mReadBuffer.at(i);
    if (b == (uchar)'\n') {
      QByteArray data = mReadBuffer.mid(iLineStart, i - iLineStart + 1);
      if (isAscii) {
        if (ui->checkBoxHex->isChecked()) {
          mLog.append(QString("<p style=\"color:#005500;\">%1(0x%2)</p>").arg(QString::fromLatin1(data), data.toHex().constData()));
        } else {
          mLog.append(QString("<p style=\"color:#005500;\">%1</p>").arg(QString::fromLatin1(data)));
        }
      } else {
        mLog.append(QString("<p style=\"color:#005500;\">0x%1(%2)</p>").arg(data.toHex().constData()).arg(data.size()));
      }
      iLineStart = i + 1;
      isAscii = true;
      needUpdate = true;
    } else if (!(b >= 0x20 && b <= 0x7f) && b != (uchar)'\r') {
      isAscii = false;
      force = true;
      break;
    }
  }

  if (force) {
    QByteArray data = mReadBuffer.mid(iLineStart);
    if (isAscii) {
      mLog.append(QString("<p>read '%1'</p>").arg(QString::fromLatin1(data)));
    } else {
      mLog.append(QString("<p>read 0x%1(%2)</p>").arg(data.toHex().constData()).arg(data.size()));
    }
    iLineStart = mReadBuffer.size();
    needUpdate = true;
  }

  if (iLineStart > 0) {
    mReadBuffer = mReadBuffer.mid(iLineStart);
  }

  if (needUpdate) {
    UpdateLog();
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
  QByteArray data = mPort->readAll();
  mReadBuffer.append(data);
  ShowData(false);
}

void MainWindow::OnReadTimeout()
{
  ShowData(true);
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
  if (ui->comboBoxPorts->currentText() != ui->comboBoxPorts->itemText(portIndex)) {
    portIndex = -1;
  }
  if (portIndex >= 0 && portIndex < mPortsList.size()) {
    mPort = new QSerialPort(mPortsList.at(portIndex), this);
  } else {
    mPort = new QSerialPort(ui->comboBoxPorts->currentText(), this);
  }

  mLog.clear();
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
  text.replace("\\r", "\r").replace("\\n", "\n");
  if (ui->checkBoxCr->isChecked() && ui->checkBoxLf->isChecked()) {
    if (!text.endsWith("\r\n")) {
      text.append("\r\n");
    }
  } else if (ui->checkBoxLf->isChecked()) {
    if (!text.endsWith("\n")) {
      text.append("\n");
    }
  } else if (ui->checkBoxCr->isChecked()) {
    if (!text.endsWith("\r")) {
      text.append("\r");
    }
  }

  QByteArray data = text.toLocal8Bit();
  if (!data.isEmpty()) {
    mPort->write(data);
    if (ui->checkBoxHex->isChecked()) {
      mLog.append(QString("<p style=\"color:blue\">%1(0x%2)</p>").arg(text, data.toHex().constData()));
    } else {
      mLog.append(QString("<p style=\"color:blue\">%1</p>").arg(text));
    }
    UpdateLog();
  }
}

void MainWindow::on_pushButtonSendHex_clicked()
{
  QString text = ui->lineEditHex->text();
  QByteArray data = QByteArray::fromHex(text.toLatin1());
  if (!data.isEmpty()) {
    mPort->write(data);
    mLog.append(QString("<p style=\"color:blue\">0x%1</p>").arg(data.toHex().constData()));
    UpdateLog();
  }
}

void MainWindow::on_pushButtonCustom_clicked()
{
  if (mPrintImage.isNull()) {
    mPrintImage = QImage(":/Icons/Test.png");
  }

  QByteArray data = QByteArray::fromHex(QByteArray("1d2a"));
  data.append((char)(mPrintImage.width()/8));
  data.append((char)(mPrintImage.height()/8));
  mPort->write(data);
  mBytesWritten += data.size();
  mLog.append(QString("<p style=\"color:blue\">0x%1</p>").arg(data.toHex().constData()));
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
