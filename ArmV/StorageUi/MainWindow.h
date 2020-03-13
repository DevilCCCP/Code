#pragma once

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>

#include <Lib/Db/Db.h>
#include <Lib/Ui/MainWindow2.h>

#include "Info.h"


DefineClassS(UpWaiter);
DefineClassS(UpInfo);

namespace Ui {
class MainWindow;
}
class FormSourceStore;

class MainWindow: public MainWindow2
{
  Ui::MainWindow*     ui;
  QIcon               mSourceIcon;

  Db&                 mDb;
  int                 mStoreTypeIndex;
  QString             mStoreTypeName;
  UpWaiter*           mUpWaiter;

  QList<FormSourceStore*> mSourceStores;

  Q_OBJECT

public:
  explicit MainWindow(Db& _Db, UpInfo* _UpInfo, QWidget* parent = 0);
  ~MainWindow();

private:
  bool ValidateSources();
  void LoadSources();

private slots:
  void on_tabWidgetMain_tabCloseRequested(int index);
  void on_actionAddSource_triggered();
  void on_tabWidgetMain_currentChanged(int index);
};

