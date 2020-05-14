#include "FormWorkSpace.h"
#include "ui_FormWorkSpace.h"


FormWorkSpace::FormWorkSpace(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormWorkSpace)
  , mMenu(nullptr)
  , mCurrentWorkSpace(0), mLastWorkSpace(0)
{
  ui->setupUi(this);
}

FormWorkSpace::~FormWorkSpace()
{
  delete ui;
}


void FormWorkSpace::SetMenu(QMenu* _Menu)
{
  mMenu = _Menu;

  for (WorkSpaceInfo& workSpaceInfo: mWorkSpaceInfoList) {
    mMenu->addAction(workSpaceInfo.Action);
  }
}

void FormWorkSpace::AddWorkSpace(QWidget* widget, const QString& name, const QString& descr)
{
  int index = mWorkSpaceInfoList.size();
  WorkSpaceInfo workSpaceInfo;
  workSpaceInfo.Widget = widget;
  workSpaceInfo.Name   = name;
  workSpaceInfo.Descr  = descr;

  ui->stackedWidgetMain->addWidget(widget);

  QAction* workAction = new QAction(QIcon(QString(":/Icons/%1.png").arg(name)), descr, this);
  workAction->setObjectName(QString("action") + name);
  workAction->setCheckable(true);
  workAction->setProperty("Index", index);
  workSpaceInfo.Action = workAction;
  if (index == 0) {
    workAction->setChecked(true);
  }
  connect(workAction, &QAction::toggled, this, &FormWorkSpace::OnWorkActionToggled);

  QToolButton* workButton = new QToolButton(ui->widgetControl);
  workButton->setObjectName(QString("toolButton") + name);
  workButton->setDefaultAction(workAction);
  workSpaceInfo.Button = workButton;

  ui->verticalLayoutToolButtons->insertWidget(index, workButton);
  mWorkSpaceInfoList.append(workSpaceInfo);

  if (mMenu) {
    mMenu->addAction(workSpaceInfo.Action);
  }
}

void FormWorkSpace::OnWorkActionToggled(bool checked)
{
  QAction* workAction = qobject_cast<QAction*>(sender());
  if (!workAction) {
    return;
  }

  int index = workAction->property("Index").toInt();

  if (checked) {
    if (mCurrentWorkSpace != index) {
      mLastWorkSpace = mCurrentWorkSpace;
      mCurrentWorkSpace = index;
    }
  } else {
    qSwap(mCurrentWorkSpace, mLastWorkSpace);
  }

  for (int i = 0; i < mWorkSpaceInfoList.size(); i++) {
    const WorkSpaceInfo& workSpaceInfo = mWorkSpaceInfoList.at(i);
    QSignalBlocker blockConnect(workSpaceInfo.Action);
    workSpaceInfo.Action->setChecked(mCurrentWorkSpace == i);

  }

  QSignalBlocker blockStackWidget(ui->stackedWidgetMain);
  ui->stackedWidgetMain->setCurrentIndex(mCurrentWorkSpace);
}
