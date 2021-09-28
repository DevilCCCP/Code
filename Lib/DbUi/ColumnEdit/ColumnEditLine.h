#pragma once

#include "ColumnEditA.h"


class QLineEdit;

class ColumnEditLine: public ColumnEditA
{
  QLineEdit* mCtrl;
  bool       mReadOnly;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditLine(bool _ReadOnly = false);
};

