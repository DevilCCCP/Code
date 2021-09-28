#pragma once

#include "ColumnEditA.h"


class QSpinBox;

class ColumnEditInt: public ColumnEditA
{
  qint64    mMinValue;
  qint64    mDefaultValue;
  qint64    mMaxValue;

  QSpinBox* mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditInt(qint64 _MinValue, qint64 _DefaultValue, qint64 _MaxValue);
};

