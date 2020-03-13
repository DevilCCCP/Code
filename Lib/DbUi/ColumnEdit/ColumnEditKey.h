#pragma once

#include <QAbstractTableModel>

#include "ColumnEditA.h"


class QComboBox;

class ColumnEditKey: public ColumnEditA
{
  QAbstractTableModel* mModel;
  QComboBox*           mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditKey(QAbstractTableModel* _Model);
};

