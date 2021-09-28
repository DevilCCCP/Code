#pragma once

#include <QAbstractTableModel>

#include "ColumnEditA.h"


class QComboBox;

class ColumnEditKey: public ColumnEditA
{
  QAbstractTableModel* mModel;
  QComboBox*           mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditKey(QAbstractTableModel* _Model);
};

