#pragma once

#include "ColumnEditA.h"


class QPlainTextEdit;

class ColumnEditText: public ColumnEditA
{
  int             mHeightLimit;
  int             mVerticalStretch;
  QPlainTextEdit* mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) Q_DECL_OVERRIDE;
  /*override */virtual bool LoadValue(const QVariant& value) Q_DECL_OVERRIDE;
  /*override */virtual bool SaveValue(QVariant& value) Q_DECL_OVERRIDE;

public:
  ColumnEditText(int _HeightLimit = 0, int _VerticalStretch = 0);
};

