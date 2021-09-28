#pragma once

#include <QObject>

#include "ColumnEditA.h"


class QPushButton;

class ColumnEditImage: public QObject, public ColumnEditA
{
  QSize                mSize;
  QPushButton*         mCtrl;

  QByteArray           mData;

  Q_OBJECT

public:
  /*override */virtual QWidget* CreateControl(QWidget* parent) override;
  /*override */virtual bool LoadValue(const QVariant& value) override;
  /*override */virtual bool SaveValue(QVariant& value) override;

private:
  void SetIcon(const QIcon& icon);

private:
  void OnPushIcon();

public:
  ColumnEditImage(int width = 32, int height = 0);
};

