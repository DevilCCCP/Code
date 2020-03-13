#pragma once

#include "ColumnEditA.h"


class QLineEdit;

class ColumnEditLine: public ColumnEditA
{
  QLineEdit* mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditLine();
};

