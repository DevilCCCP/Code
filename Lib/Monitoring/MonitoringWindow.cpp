#include <QDateTime>
#include <QPicture>
#include <QImage>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Db/Event.h>
#include <Lib/Db/Files.h>
#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <Lib/Ui/UpWaiter.h>

#include "MonitoringWindow.h"
#include "ui_MonitoringWindow.h"


void MonitoringWindow::ChangeRefreshRate(int rateMs, QAction *refreshNew)
{
  if (rateMs > 0) {
    mRefreshTimer->start(rateMs);
  } else {
    mRefreshTimer->stop();
  }
  mRefreshLast->setChecked(false);
  refreshNew->setChecked(true);
  mRefreshLast = refreshNew;
}

void MonitoringWindow::OnHasPreviewChanged(bool has)
{
  ui->dockWidgetPreview->setVisible(has);
}

void MonitoringWindow::OnPreviewEvent(qint64 fileId)
{
  QImage img = mNoPreview;
  FilesS file;
  if (fileId) {
    mCore->getFilesTable()->SelectById(fileId, file);
    if (file && !file->Data.isEmpty()) {
      QImage im = QImage::fromData(file->Data);
      if (!im.isNull()) {
        img = im;
      }
    }
  }
  ui->labelPreview->setPixmap(QPixmap::fromImage(img));
}

void MonitoringWindow::OnUpdate()
{
  if (!mLoadSchema) {
    mLoadSchema = mCore->ReloadSchema();
    ui->formSystem->ReloadState();
    ui->formSysLog->ReloadObjects();
    ui->formEvents->ReloadEvents();
  }

  bool result = false;
  if (mLoadSchema) {
    if (ui->tabWidgetMain->currentWidget() == ui->tabState) {
      result = ui->formSystem->UpdateState();
    } else {
      return;
    }
  }

  if (result) {
    OnInfo(QString::fromUtf8("Обновлено успешно %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  } else {
    OnInfo(QString::fromUtf8("Ошибка обновления %1").arg(QTime::currentTime().toString(Qt::ISODate)));
  }
}

void MonitoringWindow::OnInfo(QString info)
{
  ui->statusBar->showMessage(info);
}

void MonitoringWindow::on_actionExit_triggered()
{
  close();
}

//bool MonitoringWindow::CancelEvents(const QList<qint64>& ids)
//{
//  for (auto itr = ids.begin(); itr != ids.end(); itr++) {
//    qint64 id = *itr;
//    if (!mCore->getEventTable()->CancelEvent(id)) {
//      return false;
//    }
//  }
//  return true;
//}

void MonitoringWindow::on_actionRefresh_triggered()
{
  mLoadSchema = false;
  OnUpdate();
}

void MonitoringWindow::on_actionRefreshRate0_5_triggered()
{
  ChangeRefreshRate(500, ui->actionRefreshRate0_5);
}

void MonitoringWindow::on_actionRefreshRate1_triggered()
{
  ChangeRefreshRate(1000, ui->actionRefreshRate1);
}

void MonitoringWindow::on_actionRefreshRate2_triggered()
{
  ChangeRefreshRate(2000, ui->actionRefreshRate2);
}

void MonitoringWindow::on_actionRefreshRate5_triggered()
{
  ChangeRefreshRate(5000, ui->actionRefreshRate5);
}

void MonitoringWindow::on_actionRefreshRate30_triggered()
{
  ChangeRefreshRate(3000, ui->actionRefreshRate30);
}

void MonitoringWindow::on_actionRefreshPause_triggered()
{
  ChangeRefreshRate(0, ui->actionRefreshPause);
}

void MonitoringWindow::on_tabWidgetMain_currentChanged(int index)
{
  if (index == 2) {
    if (!mPreviewSeparator) {
      mPreviewSeparator = ui->menuView->addSeparator();
      ui->menuView->addActions(QList<QAction*>() << ui->formEvents->PreviewAction());
    }
    ui->dockWidgetPreview->setVisible(ui->formEvents->PreviewAction()->isChecked());
  } else if (mPreviewSeparator) {
    ui->menuView->removeAction(mPreviewSeparator);
    ui->menuView->removeAction(ui->formEvents->PreviewAction());
    mPreviewSeparator = nullptr;
    ui->dockWidgetPreview->setVisible(false);
  }

  if (mLoadSchema) {
    OnUpdate();
  }
}


MonitoringWindow::MonitoringWindow(Db &_Db, UpInfo* _UpInfo, bool _UseEvents, QWidget *parent)
  : MainWindow2(parent), ui(new Ui::MonitoringWindow)
  , mCore(new Core(_Db)), mUpWaiter(new UpWaiter(_UpInfo, this))
  , mLoadSchema(false)
  , mPreviewSeparator(nullptr), mNoPreview(":/Icons/No image.png")
{
  ui->setupUi(this);

  ui->tabWidgetMain->setCurrentIndex(0);
  if (!_UseEvents) {
    ui->tabWidgetMain->removeTab(2);
  }

  mRefreshTimer = new QTimer(this);
  mRefreshLast = ui->actionRefreshRate0_5;

  RegisterSaveWidget(ui->dockWidgetPreview);
  connect(ui->formEvents, &FormEvents::Preview, this, &MonitoringWindow::OnPreviewEvent);

  connect(mRefreshTimer, SIGNAL(timeout()), this, SLOT(OnUpdate()));
  on_actionRefreshRate0_5_triggered();

  Restore();

  ui->dockWidgetPreview->setVisible(false);

  connect(ui->formSysLog, &FormSysLog::Info, this, &MonitoringWindow::OnInfo);
  connect(ui->formEvents, &FormEvents::Info, this, &MonitoringWindow::OnInfo);
  connect(ui->formEvents->PreviewAction(), &QAction::triggered, this, &MonitoringWindow::OnHasPreviewChanged);

  ui->formSystem->Init(mCore.data());
  ui->formSysLog->Init(mCore.data());
  ui->formEvents->Init(mCore.data());
}

MonitoringWindow::~MonitoringWindow()
{
  delete ui;
}

