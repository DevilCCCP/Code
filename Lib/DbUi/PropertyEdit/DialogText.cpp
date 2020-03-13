#include "DialogText.h"
#include "ui_DialogText.h"


DialogText::DialogText(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogText)
{
  ui->setupUi(this);
#ifdef LANG_EN
  setWindowTitle("Edit property text");
#else
  setWindowTitle("Редактирование свойства");
#endif
}

DialogText::~DialogText()
{
  delete ui;
}


QString DialogText::Text()
{
  return ui->plainTextEditMain->toPlainText();
}

void DialogText::SetText(const QString& text)
{
  ui->plainTextEditMain->setPlainText(text);
}


void DialogText::on_buttonBox_accepted()
{
  emit Accepted();
}

void DialogText::on_buttonBox_rejected()
{
  emit Rejected();
}
