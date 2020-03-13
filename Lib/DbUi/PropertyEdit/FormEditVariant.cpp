#include <limits>

#include "FormEditVariant.h"


FormEditVariant::FormEditVariant(QWidget* parent)
  : QWidget(parent)
{
}

FormEditVariant::~FormEditVariant()
{
}


void FormEditVariant::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  Q_UNUSED(_MinValue);
  Q_UNUSED(_MaxValue);
}

void FormEditVariant::EditDone()
{
  emit Done();
}
