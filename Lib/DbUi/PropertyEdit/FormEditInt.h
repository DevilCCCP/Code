#pragma once

#include "FormEditVariant.h"


namespace Ui {
class FormEditInt;
}

class FormEditInt: public FormEditVariant
{
  Ui::FormEditInt* ui;
  qint64       mMinValue;
  qint64       mMaxValue;

  Q_OBJECT

public:
  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) Q_DECL_OVERRIDE;
  /*override */virtual void SetCurrent(const QVariant& data) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetCurrent() Q_DECL_OVERRIDE;

public:
  explicit FormEditInt(QWidget* parent = 0);
  ~FormEditInt();

private slots:
  void on_spinBoxMain_editingFinished();
};
