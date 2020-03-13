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
//  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) Q_DECL_OVERRIDE;
  /*override */virtual void SetCurrent(const QVariant& data) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetCurrent() Q_DECL_OVERRIDE;

public:
  explicit FormEditTimeRange(QWidget* parent = 0);
  ~FormEditTimeRange();

private:
  void ApplyRange(const QString& text, QTimeEdit* timeEdit);

private slots:
};
