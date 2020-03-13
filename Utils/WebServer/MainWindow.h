#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow* ui;

  QSettings*      mSettings;
  QStringList     mReplyList;
  QStringList     mWriteList;

  QTcpServer*     mTcpServer;
  QTcpSocket*     mCurrentConnection;
  QString         mLog;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void UpdateLog();
  void Write(const QString& text);
  void AddToReplyRecent();
  void AddToWriteRecent();

private:
  void OnNewConnection();
  void OnReadyRead();
  void OnDisconnected();

private slots:
  void on_pushButtonStart_clicked();
  void on_pushButtonStop_clicked();
  void on_pushButtonWrite_clicked();
  void on_pushButtonDisconnect_clicked();
};
