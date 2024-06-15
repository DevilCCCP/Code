#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include <QMap>
#include <QVector>
#include <QSerialPortInfo>
#include <QSerialPort>


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow*        ui;

  QTimer*                mReadTimer;
  QByteArray             mReadBuffer;
  QList<QSerialPortInfo> mPortsList;
  QSerialPort*           mPort;

  QMap<int, int>         mFlowControl;
  QMap<int, int>         mParity;
  QMap<int, int>         mStopBits;
  QVector<int>           mFlowControlValue;
  QVector<int>           mParityValue;
  QVector<int>           mStopBitsValue;

  QString                mLog;
  QImage                 mPrintImage;
  int                    mLineWritten;
  int                    mBytesWritten;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected:
  /*override */virtual void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;

private:
  void InitSerials();
  void WriteLine();
  void ShowData(bool force);
  void UpdateLog();

private:
  void OnReadyRead();
  void OnReadTimeout();
  void OnError(QSerialPort::SerialPortError serialPortError);
  void OnClose();
  void OnBytesWritten(qint64 bytes);
  void OnChange();

private slots:
  void on_pushButtonSelect_clicked();
  void on_pushButtonSendText_clicked();
  void on_pushButtonSendHex_clicked();
  void on_pushButtonCustom_clicked();
  void on_pushButtonClose_clicked();
  void on_pushButtonSet_clicked();
  void on_lineEditHex_returnPressed();
  void on_lineEditText_returnPressed();
};
