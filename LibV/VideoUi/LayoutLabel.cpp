#include "LayoutLabel.h"


void LayoutLabel::mousePressEvent(QMouseEvent* event)
{
  Q_UNUSED(event);

  emit Clicked();
}


LayoutLabel::LayoutLabel(QVariant _UserData, QWidget *parent)
  : QLabel(parent)
  , mUserData(_UserData)
{
}

LayoutLabel::LayoutLabel(QWidget* parent)
  : QLabel(parent)
{
}

