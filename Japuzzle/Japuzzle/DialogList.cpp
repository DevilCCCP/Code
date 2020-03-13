#include <QStandardItemModel>

#include "DialogList.h"
#include "ui_DialogList.h"
#include "Core.h"
#include "Account.h"


DialogList::DialogList(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogList)
  , mRestarted(false), mListModel(new QStandardItemModel(this))
{
  ui->setupUi(this);

  ui->toolButtonTo0->setDefaultAction(ui->actionTo0);
  ui->toolButtonTo1->setDefaultAction(ui->actionTo1);
  ui->toolButtonTo2->setDefaultAction(ui->actionTo2);
  ui->toolButtonTo3->setDefaultAction(ui->actionTo3);
  ui->toolButtonRestart->setDefaultAction(ui->actionRestart);

  ui->treeViewMain->addActions(
        QList<QAction*>()
        << ui->actionTo0 << ui->actionTo1 << ui->actionTo2 << ui->actionTo3
        << ui->actionRestart);
  ui->treeViewMain->setContextMenuPolicy(Qt::ActionsContextMenu);

  LoadList();
}

DialogList::~DialogList()
{
  delete ui;
}


void DialogList::LoadList()
{
  ui->treeViewMain->setModel(nullptr);
  mListModel->clear();
  mListModel->setHorizontalHeaderLabels(QStringList() << "Файл" << "Статус" << "Новый статус");

  const QStringList&                 puzzleDirList = qAccount->getPuzzleDirList();
  const QList<Account::PuzzleInfo>& puzzleInfoList = qAccount->getPuzzleInfoList();

  mDirItems.clear();
  QIcon dirIcon(":/Icons/Open.png");
  foreach (const QString& puzzleDir, puzzleDirList) {
    QStandardItem* dirItem = new QStandardItem(dirIcon, puzzleDir);
    mListModel->appendRow(dirItem);
    mDirItems.append(dirItem);
  }
  QIcon    waitIcon(":/Icons/Game Not Solve.png");
  QIcon    doneIcon(":/Icons/Game Solve Man.png");
  QIcon tooHardIcon(":/Icons/Game Solve Ai.png");
  QIcon tooEasyIcon(":/Icons/Game Solve Bad.png");
  foreach (const Account::PuzzleInfo& puzzleInfo, puzzleInfoList) {
    QStandardItem*   nameItem = new QStandardItem(puzzleInfo.Filename);
    QStandardItem* statusItem = new QStandardItem(Account::TypeToString(puzzleInfo.Type));
    QStandardItem* actionItem = new QStandardItem();
    nameItem->setCheckable(true);
    nameItem->setCheckState(Qt::Unchecked);
    switch (puzzleInfo.Type) {
    case Account::eWait   : nameItem->setIcon(waitIcon); break;
    case Account::eDone   : nameItem->setIcon(doneIcon); break;
    case Account::eTooHard: nameItem->setIcon(tooHardIcon); break;
    case Account::eTooEasy: nameItem->setIcon(tooEasyIcon); break;
    case Account::eFail   : break;
    }
    nameItem->setData((int)puzzleInfo.Type);
    actionItem->setData(-1);
    if (QStandardItem* dirItem = mDirItems.value(puzzleInfo.DirIndex)) {
      dirItem->appendRow(QList<QStandardItem*>() << nameItem << statusItem << actionItem);
    }
  }

  ui->treeViewMain->setModel(mListModel);
  ui->treeViewMain->expandAll();
  ui->treeViewMain->resizeColumnToContents(0);
  ui->treeViewMain->resizeColumnToContents(1);
  ui->treeViewMain->resizeColumnToContents(2);
}

void DialogList::SetAction(int type)
{
  foreach (QStandardItem* dirItem, mDirItems) {
    QModelIndex dirIndex = mListModel->indexFromItem(dirItem);
    int count = mListModel->rowCount(dirIndex);
    for (int i = 0; i < count; i++) {
      QStandardItem* item1 = mListModel->itemFromIndex(mListModel->index(i, 0, dirIndex));
      if (item1->checkState() == Qt::Checked) {
        QStandardItem* item2 = mListModel->itemFromIndex(mListModel->index(i, 2, dirIndex));
        item2->setText(Account::TypeToString((Account::EPuzzleType)type));
        item2->setData(type);
      }
    }
  }
}

void DialogList::SetTypeChecked(int type)
{
  foreach (QStandardItem* dirItem, mDirItems) {
    QModelIndex dirIndex = mListModel->indexFromItem(dirItem);
    int count = mListModel->rowCount(dirIndex);
    for (int i = 0; i < count; i++) {
      QStandardItem* item = mListModel->itemFromIndex(mListModel->index(i, 0, dirIndex));
      if (item->data().toInt() == type) {
        item->setCheckState(Qt::Checked);
      } else {
        item->setCheckState(Qt::Unchecked);
      }
    }
  }
}

void DialogList::on_actionTo0_triggered()
{
  SetAction(0);
}

void DialogList::on_actionTo1_triggered()
{
  SetAction(1);
}

void DialogList::on_actionTo2_triggered()
{
  SetAction(2);
}

void DialogList::on_actionTo3_triggered()
{
  SetAction(3);
}

void DialogList::on_buttonBox_accepted()
{
  mRestarted = ui->actionRestart->isChecked();

  foreach (QStandardItem* dirItem, mDirItems) {
    QModelIndex dirIndex = mListModel->indexFromItem(dirItem);
    int count = mListModel->rowCount(dirIndex);
    for (int i = 0; i < count; i++) {
      QStandardItem* item = mListModel->itemFromIndex(mListModel->index(i, 2, dirIndex));
      int actionType = item->data().toInt();
      if (actionType >= 0) {
        qAccount->PuzzleChangeStateTo(i, (Account::EPuzzleType)actionType);
      }
    }
  }
  if (mRestarted) {
    qCore->TakeFirstPuzzle();
  }
  qAccount->Save();
//  ui->buttonBox->accepted();
}

void DialogList::on_labelAll_linkActivated(const QString&)
{
  foreach (QStandardItem* dirItem, mDirItems) {
    QModelIndex dirIndex = mListModel->indexFromItem(dirItem);
    int count = mListModel->rowCount(dirIndex);
    for (int i = 0; i < count; i++) {
      QStandardItem* item = mListModel->itemFromIndex(mListModel->index(i, 0, dirIndex));
      item->setCheckState(Qt::Checked);
    }
  }
}

void DialogList::on_labelNone_linkActivated(const QString&)
{
  foreach (QStandardItem* dirItem, mDirItems) {
    QModelIndex dirIndex = mListModel->indexFromItem(dirItem);
    int count = mListModel->rowCount(dirIndex);
    for (int i = 0; i < count; i++) {
      QStandardItem* item = mListModel->itemFromIndex(mListModel->index(i, 0, dirIndex));
      item->setCheckState(Qt::Unchecked);
    }
  }
}

void DialogList::on_labelState0_linkActivated(const QString&)
{
  SetTypeChecked(0);
}

void DialogList::on_labelState1_linkActivated(const QString&)
{
  SetTypeChecked(1);
}

void DialogList::on_labelState2_linkActivated(const QString&)
{
  SetTypeChecked(2);
}

void DialogList::on_labelState3_linkActivated(const QString&)
{
  SetTypeChecked(3);
}

void DialogList::on_actionRestart_triggered()
{
  mRestarted = true;
}
