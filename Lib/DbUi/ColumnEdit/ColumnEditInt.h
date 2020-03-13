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
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditInt(qint64 _MinValue, qint64 _DefaultValue, qint64 _MaxValue);
};

