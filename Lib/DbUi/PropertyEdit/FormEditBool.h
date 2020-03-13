#pragma once

#include "FormEditVariant.h"


namespace Ui {
class FormEditBool;
}

class FormEditBool: public FormEditVariant
{
  Ui::FormEditBool* ui;
  QString           mFalseText;
  QString           mTrueText;

  Q_OBJECT

public:
  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) Q_DECL_OVERRIDE;
  /*override */virtual void SetCurrent(const QVariant& data) Q_DECL_OVERRIDE;
  /*override */virtual QVariant GetCurrent() Q_DECL_OVERRIDE;

public:
  explicit FormEditBool(QWidget* parent = 0);
  ~FormEditBool();

private slots:
  void on_radioButtonNo_clicked();
  void on_radioButtonYes_clicked();
};
