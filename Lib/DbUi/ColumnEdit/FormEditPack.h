#pragma once

#include <QWidget>

#include "ColumnEditA.h"


DefineClassS(FilesPackage);
class QTreeView;
class QStandardItemModel;

namespace Ui {
class FormEditPack;
}

class FormEditPack: public QWidget
{
  Ui::FormEditPack* ui;

  FilesPackageS           mFilesPackage;
  QStandardItemModel*     mModel;

  Q_OBJECT

public:
  explicit FormEditPack(QWidget* parent = 0);
  ~FormEditPack();

public:
  void SetPackage(const FilesPackageS& _FilesPackage);

private:
  void LoadModel();

private slots:
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
};
