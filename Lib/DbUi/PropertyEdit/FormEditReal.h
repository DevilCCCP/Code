#pragma once

#include "FormEditVariant.h"


namespace Ui {
class FormEditReal;
}

class FormEditReal: public FormEditVariant
{
  Ui::FormEditReal* ui;
  qreal             mMinValue;
  qreal             mMaxValue;

  Q_OBJECT

public:
  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) override;
  /*override */virtual void SetCurrent(const QVariant& data) override;
  /*override */virtual QVariant GetCurrent() override;

public:
  explicit FormEditReal(QWidget* parent = 0);
  ~FormEditReal();

private slots:
  void on_doubleSpinBoxMain_editingFinished();
};
