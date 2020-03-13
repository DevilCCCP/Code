#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QList>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Ui::MainWindow* ui;

  QSettings       mSettings;
  QString         mPath;
  int             mCurrentFrame;

  typedef QMap<QString, QLabel*> ItemsMap;
  QList<QString>  mItems;
  ItemsMap        mItemsMap;
  bool            mScaled;
  int             mIndex;
  bool            mInit;

  QTimer*         mSaveStateTimer;

  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;

private:
  void ScheduleStateSave();
  void SaveWindowState();
  void SelectPath();
  void ReloadView();
  void DrawImage();

private slots:
  void on_spinBox_valueChanged(int arg1);
  void on_spinBox_2_valueChanged(int arg1);
  void on_spinBoxIndex_valueChanged(int value);
  void on_actionBrowse_triggered();
  void on_actionSelect_triggered();
  void on_actionClear_triggered();
  void on_actionScaled_triggered(bool checked);
};

