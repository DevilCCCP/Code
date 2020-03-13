#include <QMessageBox>

#include <Lib/Common/Icon.h>
#include <Lib/Ui/UpWaiter.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


const QString GetProgramName();

MainWindow::MainWindow(Db& _Db, UpInfo* _UpInfo, QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mDb(_Db), mStoreTypeIndex(1), mUpWaiter(new UpWaiter(_UpInfo, this))
{
  Q_INIT_RESOURCE(DbUi);

  ui->setupUi(this);

  mSourceIcon = IconFromImage(":/ObjTree/rep", ":/Icons/Unlink.png");
  ui->actionAddSource->setIcon(mSourceIcon);
  ui->tabWidgetMain->setTabIcon(0, mSourceIcon);

  this->addAction(ui->actionAddSource);
  mSourceStores.append(ui->formSourceStore1);
  ui->formSourceStore1->setObjectName(QString("Источник №%1").arg(mStoreTypeIndex));

  ui->tabWidgetMain->setCurrentIndex(0);
  Restore();
}

MainWindow::~MainWindow()
{
}


bool MainWindow::ValidateSources()
{
  int cellSize = 0;
  int pageSize = 0;
  foreach (FormSourceStore* source, mSourceStores) {
    QString filename = source->Filename();
    if (filename.isEmpty()) {
      ui->labelError->setText(QString("%1 не определён").arg(source->objectName()));
      return false;
    }

    if (source->CellCount() <= 0) {
      ui->labelError->setText(QString("%1 ничего не экспортирует").arg(source->objectName()));
      return false;
    }

    if (!cellSize) {
      cellSize = source->GetCellSize();
    } else if (cellSize != source->GetCellSize()) {
      ui->labelError->setText("Размер кластера источников должен совпадать");
      return false;
    }
    if (!pageSize) {
      pageSize = source->GetPageSize();
    } else if (pageSize != source->GetPageSize()) {
      ui->labelError->setText("Размер страниц источников должен совпадать");
      return false;
    }
  }

  ui->formDestStore->SetCellSize(cellSize);
  ui->formDestStore->SetPageSize(pageSize);
  return true;
}

void MainWindow::LoadSources()
{
  int cellCount = 0;
  foreach (FormSourceStore* source, mSourceStores) {
    cellCount += source->CellCount();
  }

  QVector<ContInfo> conts;
  conts.reserve(mSourceStores.size());
  QVector<QString> names;
  names.reserve(mSourceStores.size());
  QVector<CellInfoEx> cells;
  cells.reserve(cellCount);
  int index = 0;
  foreach (FormSourceStore* source, mSourceStores) {
    source->AddCells(index, &cells);
    conts.append(source->GetContainerInfo());
    names.append(source->objectName());
    index++;
  }
  ui->formDestStore->SetSize(cellCount);
  ui->formDestStore->SetSources(conts, cells, names);
}

void MainWindow::on_tabWidgetMain_tabCloseRequested(int index)
{
  if (index >= ui->tabWidgetMain->count() - 1) {
    QMessageBox::information(this, GetProgramName(), "Конечная цель у нас одна и её нельзя изменить!");
  } else if (ui->tabWidgetMain->count() <= 2) {
    QMessageBox::information(this, GetProgramName(), "Необходим хотя бы один источник");
  } else {
    QWidget* widget = ui->tabWidgetMain->widget(index);
    foreach (QObject* child, widget->children()) {
      if (FormSourceStore* src = qobject_cast<FormSourceStore*>(child)) {
        mSourceStores.removeOne(src);
        break;
      }
    }

    ui->tabWidgetMain->removeTab(index);
  }
}

void MainWindow::on_actionAddSource_triggered()
{
  QString name = QString("Источник №%1").arg(++mStoreTypeIndex);
  QWidget* tabSource2 = new QWidget();
  tabSource2->setObjectName(QStringLiteral("tabSource2"));
  QVBoxLayout* verticalLayout_2 = new QVBoxLayout(tabSource2);
  verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
  FormSourceStore* formSourceStore2 = new FormSourceStore(tabSource2);
  formSourceStore2->setObjectName(name);

  verticalLayout_2->addWidget(formSourceStore2);

  mSourceStores.append(formSourceStore2);
  ui->tabWidgetMain->insertTab(ui->tabWidgetMain->count() - 1, tabSource2, mSourceIcon, name);
}

void MainWindow::on_tabWidgetMain_currentChanged(int index)
{
  if (index >= ui->tabWidgetMain->count() - 1) {
    if (ValidateSources()) {
      LoadSources();
      ui->formDestStore->setVisible(true);
      ui->labelError->setVisible(false);
    } else {
      ui->formDestStore->setVisible(false);
      ui->labelError->setVisible(true);
    }
  }
}
