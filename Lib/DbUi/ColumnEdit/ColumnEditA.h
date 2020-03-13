#pragma once

#include <QWidget>
#include <QVariant>

#include <Lib/Include/Common.h>


DefineClassS(ColumnEditA);

class ColumnEditA
{
public:
  /*new */virtual QWidget* CreateControl(QWidget* parent) = 0;
  /*new */virtual bool LoadValue(const QVariant& value) = 0;
  /*new */virtual bool SaveValue(QVariant& value) = 0;

public:
  /*new */virtual ~ColumnEditA() { }
};
