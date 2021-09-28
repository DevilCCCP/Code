#pragma once

#include "ColumnEditA.h"


class QDoubleSpinBox;

class ColumnEditReal: public ColumnEditA
{
  qreal           mMinValue;
  qreal           mDefaultValue;
  qreal           mMaxValue;

  QDoubleSpinBox* mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditReal(qreal _MinValue, qreal _DefaultValue, qreal _MaxValue);
};

