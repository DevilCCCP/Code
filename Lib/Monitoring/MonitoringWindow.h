#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include <Lib/Db/Db.h>
#include <Lib/Ui/MainWindow2.h>

#include "Core.h"


DefineClassS(Core);
DefineClassS(UpWaiter);
DefineClassS(UpInfo);

namespace Ui {
class MonitoringWindow;
}

typedef QStandardItem* StateMl;
typedef QPair<QBrush, QBrush> ItemView;

class MonitoringWindow: public MainWindow2
{
  Ui::MonitoringWindow*     ui;

  CoreS               mCore;
  UpWaiter*           mUpWaiter;

  QTimer*             mRefreshTimer;
  QAction*            mRefreshLast;
  bool                mLoadSchema;

  QAction*            mPreviewSeparator;
  QImage              mNoPreview;

  Q_OBJECT

private:
  void ChangeRefreshRate(int rateMs, QAction* refreshNew);
  void OnHasPreviewChanged(bool has);
  void OnPreviewEvent(qint64 fileId);

public slots:
  void OnUpdate();
  void OnInfo(QString info);

private slots:
  void on_actionExit_triggered();

  void on_actionRefresh_triggered();
  void on_actionRefreshRate0_5_triggered();
  void on_actionRefreshRate1_triggered();
  void on_actionRefreshRate2_triggered();
  void on_actionRefreshRate5_triggered();
  void on_actionRefreshRate30_triggered();
  void on_actionRefreshPause_triggered();
  void on_tabWidgetMain_currentChanged(int index);

public:
  explicit MonitoringWindow(Db& _Db, UpInfo* _UpInfo, bool _UseEvents, QWidget* parent = nullptr);
  ~MonitoringWindow();
};

