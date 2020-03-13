#pragma once

#include <QAbstractItemModel>
#include <QVector>
#include <QIcon>

#include <Lib/Include/Common.h>


DefineStructS(AccountInfo);

class AccountModel: public QAbstractItemModel
{
  QVector<AccountInfoS> mAccountsInfo;
  AccountInfoS          mNewAccountInfo;
  QIcon                 mAccountIcon;

public:
  /*override */virtual int columnCount(const QModelIndex& = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex index(int row, int column, const QModelIndex& = QModelIndex()) const Q_DECL_OVERRIDE;
  /*override */virtual QModelIndex parent(const QModelIndex&) const Q_DECL_OVERRIDE;
  /*override */virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

  /*override */virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;
  /*override */virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

public:
  void SetList(const QVector<AccountInfoS>& _AccountsInfo);
  AccountInfoS GetItem(int index) const;

public:
  explicit AccountModel(QObject* parent = 0);
};
