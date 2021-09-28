#pragma once

#include "ColumnEditA.h"


class QPlainTextEdit;

class ColumnEditText: public ColumnEditA
{
  int             mHeightLimit;
  int             mVerticalStretch;
  QPlainTextEdit* mCtrl;

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

public:
  ColumnEditText(int _HeightLimit = 0, int _VerticalStretch = 0);
};

