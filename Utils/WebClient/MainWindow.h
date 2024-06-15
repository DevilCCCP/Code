#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QNetworkAccessManager>


namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow* ui;

  QSettings*      mSettings;
  QStringList     mOnceList;
  QStringList     mPeriodList;
  int             mProxyType;
  QString         mProxyUri;

  QNetworkAccessManager* mNetManager;
  QTimer*                mTimer;
  QString                mLog;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

private:
  void UpdateLog();
  void AddToOnceRecent();
  void AddToPeriodRecent();
  bool SetProxy();

private:
  void OnFinished(QNetworkReply* netReply);
  void OnSendTime();

private slots:
  void on_pushButtonSend_clicked();
  void on_pushButtonStart_clicked();
  void on_pushButtonStop_clicked();
};
