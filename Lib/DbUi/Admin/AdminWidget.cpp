#include "AdminWidget.h"


void AdminWidget::Activate()
{
  if (!mLoad) {
    Prepare();
    mLoad = true;
  }
  mIsActive = true;

  Activated();
}

void AdminWidget::Deactivate()
{
  mIsActive = false;

  Deactivated();
}

void AdminWidget::Prepare()
{
}

void AdminWidget::Activated()
{
}

void AdminWidget::Deactivated()
{
}

AdminWidget::AdminWidget(QWidget* parent)
  : QWidget(parent)
  , mIsActive(false), mLoad(false)
{
}
