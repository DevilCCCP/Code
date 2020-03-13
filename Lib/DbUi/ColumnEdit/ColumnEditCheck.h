#pragma once

#include "ColumnEditA.h"


class QCheckBox;

class ColumnEditCheck: public ColumnEditA
{
  const QString mCheckText;

  QCheckBox*    mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditCheck(const QString& _CheckText);
};

