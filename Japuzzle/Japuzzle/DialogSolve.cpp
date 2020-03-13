#include "DialogSolve.h"
#include "ui_DialogSolve.h"


DialogSolve::DialogSolve(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogSolve)
{
  ui->setupUi(this);
}

DialogSolve::~DialogSolve()
{
  delete ui;
}


void DialogSolve::ChangeCount(int value, int maximum)
{
  ui->progressBarMain->setMaximum(maximum);
  ui->progressBarMain->setValue(value);
}

void DialogSolve::on_pushButtonCancel_clicked()
{
  hide();
}
