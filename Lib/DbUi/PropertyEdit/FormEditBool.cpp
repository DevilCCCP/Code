#include "FormEditBool.h"
#include "ui_FormEditBool.h"


void FormEditBool::SetValues(const QString& _MinValue, const QString& _MaxValue)
{
  mFalseText = _MinValue;
  mTrueText = _MaxValue;

  ui->radioButtonNo->setText(mFalseText);
  ui->radioButtonYes->setText(mTrueText);
}

void FormEditBool::SetCurrent(const QVariant& data)
{
  bool ok;
  int yes = data.toInt(&ok);
  if (ok) {
    if (yes) {
      ui->radioButtonYes->setChecked(true);
    } else {
      ui->radioButtonNo->setChecked(true);
    }
  }
  QString text = data.toString();
  if (text == mTrueText) {
    ui->radioButtonYes->setChecked(true);
  } else if (text == mFalseText) {
    ui->radioButtonNo->setChecked(true);
  }
}

QVariant FormEditBool::GetCurrent()
{
  if (ui->radioButtonYes->isChecked()) {
    return QVariant(1);
  } else if (ui->radioButtonNo->isChecked()) {
    return QVariant(0);
  }
  return QVariant();
}


FormEditBool::FormEditBool(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditBool)
{
  ui->setupUi(this);
}

FormEditBool::~FormEditBool()
{
  delete ui;
}

void FormEditBool::on_radioButtonNo_clicked()
{
  EditDone();
}

void FormEditBool::on_radioButtonYes_clicked()
{
  EditDone();
}
