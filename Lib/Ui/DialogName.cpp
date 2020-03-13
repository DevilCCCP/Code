#include <QPushButton>

#include "DialogName.h"
#include "ui_DialogName.h"


DialogName::DialogName(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogName)
{
  ui->setupUi(this);

  on_lineEditName_textEdited(QString());
}

DialogName::~DialogName()
{
  delete ui;
}


QString DialogName::Open(QWidget* parent, const QString& tytle, const QString& caption, QString name)
{
  DialogName* dialog = new DialogName(parent);
  dialog->SetTytle(tytle);
  dialog->SetCaption(caption);
  dialog->SetName(name);
  int result = dialog->exec();
  if (result == QDialog::Accepted) {
    return dialog->Name();
  }
  return QString();
}

QString DialogName::Tytle() const
{
  return windowTitle();
}

QString DialogName::Caption() const
{
  return ui->labelCaption->text();
}

QString DialogName::Name() const
{
  return ui->lineEditName->text();
}

void DialogName::SetTytle(const QString& tytle)
{
  setWindowTitle(tytle);
}

void DialogName::SetCaption(const QString& caption)
{
  ui->labelCaption->setText(caption);
}

void DialogName::SetName(const QString& name)
{
  ui->lineEditName->setText(name);
  on_lineEditName_textEdited(name);
}

void DialogName::on_lineEditName_textEdited(const QString& text)
{
  ui->buttonBoxMain->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void DialogName::on_buttonBoxMain_accepted()
{
  accept();
}

void DialogName::on_buttonBoxMain_rejected()
{
  SetName(QString());

  reject();
}
