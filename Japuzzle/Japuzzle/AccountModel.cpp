#include "AccountModel.h"
#include "AccountInfo.h"


int AccountModel::columnCount(const QModelIndex&) const
{
  return 1;
}

QVariant AccountModel::data(const QModelIndex& index, int role) const
{
  int ind = index.row();
  const AccountInfoS& info = mAccountsInfo.value(ind);
  if (info) {
    switch (role) {
    case Qt::DisplayRole   : return info->ViewName;
    case Qt::EditRole      : return info->Name;
    case Qt::DecorationRole: return mAccountIcon;
    }
  }
  return QVariant();
}

QModelIndex AccountModel::index(int row, int column, const QModelIndex&) const
{
  return createIndex(row, column);
}

QModelIndex AccountModel::parent(const QModelIndex&) const
{
  return QModelIndex();
}

int AccountModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return mAccountsInfo.size();
  }
  return 0;
}

Qt::ItemFlags AccountModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  AccountInfoS info = GetItem(index.row());
  if (info && info->Name.isEmpty()) {
    flag |= Qt::ItemIsEditable;
  }
  return flag;
}

bool AccountModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role == Qt::EditRole) {
    AccountInfoS info = GetItem(index.row());
    if (info->Name.isEmpty()) {
      QString name = value.toString();
      if (!name.isEmpty()) {
        mNewAccountInfo->Name = mNewAccountInfo->ViewName = name;
        emit dataChanged(index, index);
        return true;
      }
    }
  }
  return false;
}

void AccountModel::SetList(const QVector<AccountInfoS>& _AccountsInfo)
{
  mAccountsInfo.clear();
  mAccountsInfo.append(_AccountsInfo);
  mAccountsInfo.append(mNewAccountInfo);
}

AccountInfoS AccountModel::GetItem(int index) const
{
  return mAccountsInfo.value(index);
}


AccountModel::AccountModel(QObject* parent)
  : QAbstractItemModel(parent)
  , mNewAccountInfo(new AccountInfo()), mAccountIcon(QIcon(":/Icons/User.png"))
{
  mNewAccountInfo->ViewName = "Новый пользователь";
  mNewAccountInfo->Existed  = false;
}

