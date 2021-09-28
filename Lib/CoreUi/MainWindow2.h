#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QWidget>

#include <Lib/Include/Common.h>


class QSettings;
class QTimer;

class MainWindow2: public QMainWindow
{
  QSettings*          mSettings;
  QTimer*             mSaveStateTimer;
  bool                mRestored;

  QList<QWidget*>     mSaveWidgets;
  QList<QSplitter*>   mSaveSplitters;

  Q_OBJECT

public:
  MainWindow2(QWidget* parent = 0);

protected:
  QSettings* GetSettings() { return mSettings; }

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
  /*override */virtual void resizeEvent(QResizeEvent* event) override;
  /*override */virtual void moveEvent(QMoveEvent* event) override;
};

