#pragma once

#include <QMainWindow>
#include <QElapsedTimer>

#include "MulInfo.h"


namespace Ui {
class MainWindow;
}

class MulInfoModel;
class QSettings;

class MainWindow: public QMainWindow
{
  Ui::MainWindow*  ui;

  QString          mUser;
  QSettings*       mUserInfo;
  bool             mPlaying;
  bool             mStopping;

  QVector<MulInfo> mResult;
  MulInfoModel*    mResultModel;

  QElapsedTimer    mPlayElapsed;
  QTimer*          mPlayTimer;
  int              mPlayIndex;
  int              mPlaySeries;
  QVector<int>     mPlayPlan;

  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  ~MainWindow();

protected:
  virtual void showEvent(QShowEvent *event) override;
  virtual void closeEvent(QCloseEvent* event) override;

private:
  bool Init();
  void LoadResult(MulInfo* mulInfo);
  void SaveResult(const MulInfo* mulInfo);
  void SetResultOk(MulInfo* mulInfo);
  void SetResultFail(MulInfo* mulInfo);
  bool Save();
  void Play();
  void Stop();
  void PlayNext();
  void PlayDone();
  void PlayHalt();

  void UpdateActions();
  void ReloadModel();

signals:
  void Login();

private:
  void OnLogin();
  void OnTimeout();

private slots:
  void on_actionExit_triggered();
  void on_actionStart_triggered();
  void on_actionStop_triggered();
  void on_lineEditSolve_returnPressed();
  void on_pushButtonPlayOk_clicked();
};
