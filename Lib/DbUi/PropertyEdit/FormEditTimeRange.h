#pragma once

#include "FormEditVariant.h"
#include "DialogText.h"


namespace Ui {
class FormEditTimeRange;
}

class QTimeEdit;

class FormEditTimeRange: public FormEditVariant
{
  Ui::FormEditTimeRange* ui;

  Q_OBJECT

public:
//  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) override;
  /*override */virtual void SetCurrent(const QVariant& data) override;
  /*override */virtual QVariant GetCurrent() override;

public:
  explicit FormEditTimeRange(QWidget* parent = 0);
  ~FormEditTimeRange();

private:
  void ApplyRange(const QString& text, QTimeEdit* timeEdit);

private slots:
};
