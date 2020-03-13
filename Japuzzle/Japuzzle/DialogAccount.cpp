#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>

#include "DialogAccount.h"
#include "ui_DialogAccount.h"
#include "Core.h"
#include "AccountModel.h"
#include "AccountInfo.h"
#include "Account.h"


DialogAccount::DialogAccount(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogAccount)
  , mAccountModel(new AccountModel(this))
{
  ui->setupUi(this);

  ui->treeViewMain->setModel(mAccountModel);
  ui->buttonBox->setFocus();

  LoadList();

  connect(ui->treeViewMain->selectionModel(), &QItemSelectionModel::selectionChanged
          , this, &DialogAccount::OnSelectionChanged);
  connect(mAccountModel, &AccountModel::dataChanged
          , this, &DialogAccount::OnSelectionChanged);
  connect(ui->pushButtonDelete, &QPushButton::clicked
          , this, &DialogAccount::OnRemoveClicked);
}

DialogAccount::~DialogAccount()
{
  delete ui;
}


AccountInfoS DialogAccount::CurrentAccount()
{
  return mCurrentIndex.isValid()? mAccountModel->GetItem(mCurrentIndex.row()): AccountInfoS();
}

void DialogAccount::LoadList()
{
  mAccountModel->SetList(qCore->getAccountsInfo());
  for (int i = 0; i < qCore->getAccountsInfo().size(); i++) {
    QToolButton* button = new QToolButton(this);
    button->setProperty("ind", i + 1);
    button->setIcon(QIcon(":/Icons/Remove.png"));
    button->setAutoRaise(true);
    ui->treeViewMain->setIndexWidget(mAccountModel->index(i, 1), button);

    connect(button, &QToolButton::clicked, this, &DialogAccount::OnRemoveClicked);
  }

  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

bool DialogAccount::IsCurrentValid()
{
  AccountInfoS account = CurrentAccount();
  return account && !account->Name.isEmpty();
}

bool DialogAccount::IsCurrentAcceptable()
{
  AccountInfoS account = CurrentAccount();
  if (account->Existed && qCore->TestAccount(CurrentAccount())) {
    return true;
  }
  return false;
}

bool DialogAccount::IsCurrentCreatable()
{
  AccountInfoS account = CurrentAccount();
  return account && !account->Name.isEmpty() && !account->Existed;
}

bool DialogAccount::RemoveAccount(int index)
{
  if (index < 0 || index >= qCore->getAccountsInfo().size()) {
    return false;
  }

  if (qAccount && qCore->getAccountsInfo().at(index)->Name == qAccount->getName()) {
    QMessageBox::information(
          this, qCore->getProgramName(), "Невозможно удалить текущую учётную запись.");
    return false;
  }

  QMessageBox::StandardButton b = QMessageBox::information(
        this, qCore->getProgramName(), "Вы собираетесь удалить учётную запись.\nНеобходимо подтверждение действия"
        , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (b != QMessageBox::Yes) {
    return false;
  }
  if (!qCore->RemoveAccount(index)) {
    return false;
  }

  mAccountModel->SetList(qCore->getAccountsInfo());
  OnSelectionChanged();
  return true;
}

void DialogAccount::OnSelectionChanged()
{
  auto selectedIndexList = ui->treeViewMain->selectionModel()->selectedRows();
  mCurrentIndex = !selectedIndexList.isEmpty()? selectedIndexList.first(): QModelIndex();
  bool acceptable = IsCurrentAcceptable();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptable || IsCurrentCreatable());
  ui->pushButtonDelete->setEnabled(acceptable);
}

void DialogAccount::OnRemoveClicked()
{
  QToolButton* button = qobject_cast<QToolButton*>(sender());
  if (button) {
    int ind = button->property("ind").toInt() - 1;
    RemoveAccount(ind);
  } else {
    auto selectedIndexList = ui->treeViewMain->selectionModel()->selectedRows();
    auto selectedIndex = !selectedIndexList.isEmpty()? selectedIndexList.first(): QModelIndex();
    if (selectedIndex.isValid()) {
      RemoveAccount(selectedIndex.row());
    }
  }
}

void DialogAccount::on_treeViewMain_doubleClicked(const QModelIndex& index)
{
  if (index.isValid()) {
    mCurrentIndex = index;
    if (IsCurrentAcceptable() || IsCurrentCreatable()) {
      accept();
    }
  }
}
