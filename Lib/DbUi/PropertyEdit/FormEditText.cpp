#include <QFocusEvent>

#include "FormEditText.h"
#include "DialogText.h"
#include "QLineEdit2.h"
#include "ui_FormEditText.h"


void FormEditText::SetCurrent(const QVariant& data)
{
  QString text = data.toString();
  ui->lineEditMain->setText(text);
}

QVariant FormEditText::GetCurrent()
{
  return QVariant(ui->lineEditMain->text());
}

void FormEditText::focusOutEvent(QFocusEvent* event)
{
  if (!mExec) {
    EditDone();
  }

  QWidget::focusOutEvent(event);
}


FormEditText::FormEditText(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditText)
  , mDialogText(nullptr), mExec(false)
{
  ui->setupUi(this);

  setFocusProxy(ui->lineEditMain);
  connect(ui->lineEditMain, &QLineEdit2::FocusLost, this, &FormEditText::OnFocusLost);
}

FormEditText::~FormEditText()
{
  delete ui;
}

void FormEditText::OnAccepted()
{
  ui->lineEditMain->setText(mDialogText->Text());

  mDialogText->deleteLater();
  mExec = false;
  ui->lineEditMain->setFocus();
}

void FormEditText::OnRejected()
{
  mDialogText->deleteLater();
  mExec = false;
  ui->lineEditMain->setFocus();
}

void FormEditText::OnFocusLost()
{
  if (!mExec) {
    EditDone();
  }
}


void FormEditText::on_toolButtonEditor_clicked()
{
  mDialogText = new DialogText(this);
  mDialogText->SetText(ui->lineEditMain->text());

  connect(mDialogText, &DialogText::Accepted, this, &FormEditText::OnAccepted);
  connect(mDialogText, &DialogText::Rejected, this, &FormEditText::OnRejected);

  mExec = true;
  mDialogText->exec();
}

void FormEditText::on_lineEditMain_returnPressed()
{
  EditDone();
}
