#pragma once

#include <QDialog>
#include <QModelIndex>

#include <Lib/Include/Common.h>


DefineClassS(AccountModel);
DefineStructS(AccountInfo);
namespace Ui {
class DialogAccount;
}

class DialogAccount: public QDialog
{
  Ui::DialogAccount* ui;
  AccountModel*      mAccountModel;

  QModelIndex        mCurrentIndex;

  Q_OBJECT

public:
  explicit DialogAccount(QWidget* parent = 0);
  ~DialogAccount();

public:
  AccountInfoS CurrentAccount();

private:
  void LoadList();
  bool IsCurrentValid();
  bool IsCurrentAcceptable();
  bool IsCurrentCreatable();
  bool RemoveAccount(int index);

private:
  void OnSelectionChanged();
  void OnRemoveClicked();

private slots:
  void on_treeViewMain_doubleClicked(const QModelIndex& index);
};
