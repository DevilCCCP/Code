#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QWidget>

#include <Lib/Include/Common.h>


DefineClassS(QSettings);
DefineClassS(QTimer);

class MainWindow2: public QMainWindow
{
  QSettings*          mSettings;
  QTimer*             mSaveStateTimer;
  bool                mRestored;
  bool                mHaveChanges;

  QList<QWidget*>     mSaveWidgets;
  QList<QSplitter*>   mSaveSplitters;

  Q_OBJECT

public:
  MainWindow2(QWidget* parent = 0);

protected:
  bool Restore();

  template <typename Func1>
  inline bool RegisterSaveWidgetA(typename QtPrivate::FunctionPointer<Func1>::Object* widget, Func1 signal)
  {
    connect(widget, signal, this, &MainWindow2::ScheduleStateSave);
    if (QWidget* widgetBase = dynamic_cast<QWidget*>(widget)) {
      return RegisterSaveWidget(widgetBase);
    } else {
      return false;
    }
  }

  bool RegisterSaveWidget(QWidget* widget);
  bool RegisterSaveSplitter(QSplitter* splitter);

private:
  void ScheduleSplitterSave(int pos, int ind);
  void ScheduleStateSave();
  void SaveWindowState();

protected:
  /*override */virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
};

