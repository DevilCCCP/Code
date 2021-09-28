#pragma once

#include "TableTModel.h"


class EventModel: public TableModel
{
  ObjectTableS        mObjectTable;
  EventTypeTableS     mEventTypeTable;
  QStringList         mHeaders;

public:
  /*override */virtual int rowCount(const QModelIndex &parent) const override;
  /*override */virtual int columnCount(const QModelIndex &parent) const override;
  /*override */virtual QVariant data(const QModelIndex &index, int role) const override;
  /*override */virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
//  virtual/*override */Qt::ItemFlags flags(const QModelIndex &index) const;
//  virtual/*override */bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
//  virtual/*override */bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
//  virtual/*override */bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
  /*override */virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

public:
  EventModel(const ObjectTableS& _ObjectTable, const EventTypeTableS& _EventTypeTable, const QStringList& _Headers = QStringList(), QObject *parent = 0);
};

