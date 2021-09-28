#pragma once

#include <QScrollArea>


class QScrollAreaZoom: public QScrollArea
{
  Q_OBJECT

protected:
  /*override */virtual void wheelEvent(QWheelEvent* event) override;

public:
  explicit QScrollAreaZoom(QWidget* parent = 0);

signals:

public slots:
};
