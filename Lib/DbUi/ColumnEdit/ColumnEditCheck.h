#pragma once

#include "ColumnEditA.h"


class QCheckBox;

class ColumnEditCheck: public ColumnEditA
{
  const QString mCheckText;

  QCheckBox*    mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditCheck(const QString& _CheckText);
};

