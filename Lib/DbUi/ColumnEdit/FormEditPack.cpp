#include <QTreeView>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>

#include <Lib/Common/FilesPackage.h>

#include "FormEditPack.h"
#include "ui_FormEditPack.h"


FormEditPack::FormEditPack(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormEditPack)
{
  ui->setupUi(this);

  mModel = new QStandardItemModel(this);
  ui->toolButtonOpen->setDefaultAction(ui->actionOpen);
  ui->toolButtonSave->setDefaultAction(ui->actionSave);
  ui->treeViewPack->addAction(ui->actionOpen);
  ui->treeViewPack->addAction(ui->actionSave);
  ui->treeViewPack->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->actionOpen->setEnabled(false);
  ui->actionSave->setEnabled(false);
}

FormEditPack::~FormEditPack()
{
  delete ui;
}


void FormEditPack::SetPackage(const FilesPackageS& _FilesPackage)
{
  mFilesPackage = _FilesPackage;
  LoadModel();

  ui->actionOpen->setEnabled(true);
  ui->actionSave->setEnabled(mFilesPackage->IsValid());
}

void FormEditPack::LoadModel()
{
  ui->treeViewPack->setModel(nullptr);
  mModel->clear();
  mModel->setHorizontalHeaderLabels(QStringList(tr("Name")));

  if (!mFilesPackage->IsValid()) {
    QStandardItem* rootItem = new QStandardItem(QString("<%1>").arg(mFilesPackage->ErrorString()));
    mModel->appendRow(rootItem);
    ui->treeViewPack->setModel(mModel);
    ui->treeViewPack->expandAll();
    return;
  }

  QStringList dirList;
  mFilesPackage->GetDirList(dirList);

  std::sort(dirList.begin(), dirList.end());
  QMap<QString, QStandardItem*> dirItemList;
  QIcon folderIcon(":/Icons/Folder.png");
  QStandardItem* rootItem = new QStandardItem(folderIcon, "/");
  mModel->appendRow(rootItem);
  foreach (const QString& dirPath, dirList) {
    QStandardItem* item = new QStandardItem(folderIcon, dirPath);
    dirItemList[dirPath] = item;
    rootItem->appendRow(item);
  }

  QStringList fileList;
  mFilesPackage->GetFileList(fileList);

  QIcon fileIcon(":/Icons/File.png");
  foreach (const QString& filePath, fileList) {
    QFileInfo file(filePath);
    QDir dir = file.dir();
    QString dirPath = dir.path();
    auto itr = dirItemList.find(dirPath);
    if (itr != dirItemList.end()) {
      QStandardItem* item = new QStandardItem(fileIcon, dir.relativeFilePath(file.fileName()));
      itr.value()->appendRow(item);
    } else {
      QStandardItem* item = new QStandardItem(fileIcon, filePath);
      rootItem->appendRow(item);
    }
  }
  ui->treeViewPack->setModel(mModel);
  ui->treeViewPack->expandAll();
}

void FormEditPack::on_actionOpen_triggered()
{
  QString dirPath = QFileDialog::getExistingDirectory(this);
  if (!dirPath.isEmpty()) {
    bool ok = mFilesPackage->PackDir(dirPath);
    LoadModel();
    ui->actionSave->setEnabled(ok);
  }
}

void FormEditPack::on_actionSave_triggered()
{
  QString dirPath = QFileDialog::getExistingDirectory(this);
  if (!dirPath.isEmpty()) {
    bool ok = mFilesPackage->UnPackDir(dirPath);
    if (!ok) {
      QMessageBox::warning(this, tr("Save package"), tr("Save package fail"));
    }
  }
}
