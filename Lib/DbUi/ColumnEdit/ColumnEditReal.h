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
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditReal(qreal _MinValue, qreal _DefaultValue, qreal _MaxValue);
};

