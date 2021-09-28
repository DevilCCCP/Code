#pragma once

#include <QStandardItemModel>


class PropertyItemModel: public QStandardItemModel
{

public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

public:
  explicit PropertyItemModel(QObject *parent = 0);
};

