#pragma once

#include <QLabel>
#include <QVariant>

#include <Lib/Include/Common.h>


class LayoutLabel: public QLabel
{
  PROPERTY_GET_SET(QVariant, UserData)

  Q_OBJECT

protected:
  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

signals:
  void Clicked();

public slots:

public:
  explicit LayoutLabel(QVariant _UserData, QWidget* parent = 0);
  explicit LayoutLabel(QWidget* parent = 0);
};

typedef QList<LayoutLabel*> LayoutList;
typedef QList<LayoutList> LayoutMatrix;
