#pragma once

#include <QMainWindow>
#include <QMap>
#include <QDate>
#include <QElapsedTimer>


class QSettings;
class QSystemTrayIcon;
namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow
{
  Ui::MainWindow*  ui;

  QSystemTrayIcon* mTrayIcon;
  int              mDayType;
  QVector<int>     mCommonLimits;
  QMap<QDate, int> mExceptionDayMap;

  QSettings*       mTodaySettings;
  int              mTodayLimit;
  int              mTodayUsed;
  int              mTodayLast;
  int              mLastPercent;
  QElapsedTimer    mUseTimer;
  QTimer*          mCalcTimer;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent* event) override;

private slots:
  void on_buttonBoxMain_accepted();

private:
  void LoadSettings();
  void Calc();

private:
  void OnTrayActivated();
  void OnTimeout();
};
