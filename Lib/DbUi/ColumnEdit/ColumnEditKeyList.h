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
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditKeyList(QAbstractTableModel* _Model);
};

