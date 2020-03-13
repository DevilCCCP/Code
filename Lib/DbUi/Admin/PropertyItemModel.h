#pragma once

#include <QStandardItemModel>


class PropertyItemModel: public QStandardItemModel
{

public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

public:
  explicit PropertyItemModel(QObject *parent = 0);
};

