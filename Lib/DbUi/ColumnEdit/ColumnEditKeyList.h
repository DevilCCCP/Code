#pragma once

#include <QAbstractTableModel>

#include "ColumnEditA.h"


class FormKeyList;
class QStandardItemModel;
class QStandardItem;

class ColumnEditKeyList: public ColumnEditA
{
  QAbstractTableModel* mModel;
  FormKeyList*         mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditKeyList(QAbstractTableModel* _Model);
};

