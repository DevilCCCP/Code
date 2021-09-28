#pragma once

#include "ColumnEditA.h"


class QComboBox;

class ColumnEditSwitch: public ColumnEditA
{
  const QStringList mSwitchText;

  QComboBox*        mCtrl;
  QMap<int, int>    mValueIndexMap;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditSwitch(const QStringList& _SwitchText);
};

