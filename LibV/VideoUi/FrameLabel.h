#pragma once

#include <QWidget>

#include <LibV/Include/Frame.h>


class FrameLabel: public QWidget
{
  FrameS          mFrame;

public:
  FrameLabel(QWidget* parent = nullptr);

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) override;

public:
  void SetFrame(const FrameS& _Frame);
};
