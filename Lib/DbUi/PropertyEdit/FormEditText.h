#pragma once

#include "FormEditVariant.h"
#include "DialogText.h"


namespace Ui {
class FormEditText;
}

class FormEditText: public FormEditVariant
{
  Ui::FormEditText* ui;

  DialogText*       mDialogText;
  bool              mExec;

  Q_OBJECT

public:
//  /*override */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue) override;
  /*override */virtual void SetCurrent(const QVariant& data) override;
  /*override */virtual QVariant GetCurrent() override;

protected:
  /*override */virtual void focusOutEvent(QFocusEvent* event) override;

public:
  explicit FormEditText(QWidget* parent = 0);
  ~FormEditText();

public slots:
  void OnAccepted();
  void OnRejected();
  void OnFocusLost();

private slots:
  void on_toolButtonEditor_clicked();
  void on_lineEditMain_returnPressed();
};
