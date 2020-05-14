#pragma once

#include <QWidget>
#include <QMenu>
#include <QToolButton>
#include <QAction>


namespace Ui {
class FormWorkSpace;
}

class FormWorkSpace: public QWidget
{
  Ui::FormWorkSpace*   ui;

  QMenu*               mMenu;

  struct WorkSpaceInfo {
    QWidget*     Widget;
    QString      Name;
    QString      Descr;
    QAction*     Action;
    QToolButton* Button;
  };
  QList<WorkSpaceInfo> mWorkSpaceInfoList;
  int                  mCurrentWorkSpace;
  int                  mLastWorkSpace;

  Q_OBJECT

public:
  explicit FormWorkSpace(QWidget* parent = 0);
  ~FormWorkSpace();

public:
  void SetMenu(QMenu* _Menu);

public:
  void AddWorkSpace(QWidget* widget, const QString& name, const QString& descr);

private:
  void OnWorkActionToggled(bool checked);
};
