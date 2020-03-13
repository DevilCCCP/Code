#pragma once

#include <QAbstractTableModel>

#include "MulInfo.h"


class MulInfoModel: public QAbstractTableModel
{
  QVector<MulInfo>    mData;

public:
  /*override */virtual int rowCount(const QModelIndex &parent) const override;
  /*override */virtual int columnCount(const QModelIndex &parent) const override;
  /*override */virtual QVariant data(const QModelIndex &index, int role) const override;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//  virtual/*override */Qt::ItemFlags flags(const QModelIndex &index) const;
//  virtual/*override */bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
//  virtual/*override */bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
//  virtual/*override */bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
//  /*override */virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

public:
  MulInfoModel(const QVector<MulInfo>& _Data, QObject* parent = 0);
};

