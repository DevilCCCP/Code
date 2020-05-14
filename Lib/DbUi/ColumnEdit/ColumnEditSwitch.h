#pragma once

#include "ColumnEditA.h"


class QComboBox;

class ColumnEditSwitch: public ColumnEditA
{
  const QStringList mSwitchText;

  QComboBox*        mCtrl;
  QMap<int, int>    mValueIndexMap;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditSwitch(const QStringList& _SwitchText);
};

