#include "DialogSolve.h"
#include "ui_DialogSolve.h"
#include "Core.h"
#include "Account.h"
#include "Puzzle.h"


DialogSolve::DialogSolve(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogSolve)
{
  ui->setupUi(this);
}

DialogSolve::~DialogSolve()
{
  delete ui;
}


void DialogSolve::Init(Account* account, Puzzle* puzzle)
{
  mAccount = account;
  mPuzzle  = puzzle;

  ui->progressBarMain->setMaximum(mPuzzle->getWidth() * mPuzzle->getHeight());
  ui->progressBarMain->setValue(mPuzzle->Count());
  ui->progressBarProp->setValue(0);

  QSignalBlocker block(ui->checkBoxAuto);
  ui->checkBoxAuto->setChecked(qAccount->getAutoOpenPropEx());

  UpdateSolveState(true);
}

void DialogSolve::UpdateSolveState(bool ready)
{
  ui->widgetParams->setVisible(ready);
  ui->widgetSolve->setVisible(!ready);
}

void DialogSolve::OnSolveChanged(int solved, int prop)
{
  ui->progressBarMain->setValue(solved);
  ui->progressBarProp->setValue(prop);
}

void DialogSolve::OnSolveDone(bool result)
{
  UpdateSolveState(true);

  if (result != 0) {
    hide();
  }
}

void DialogSolve::on_pushButtonCancel_clicked()
{
  emit StopSolve();
}

void DialogSolve::on_checkBoxAuto_toggled(bool checked)
{
  qAccount->mAutoOpenPropEx = checked;
  qAccount->Save();
}

void DialogSolve::on_pushButtonStart_clicked()
{
  int maxProp = ui->spinBoxMaxProp->value();
  ui->progressBarProp->setMaximum(maxProp);
  ui->progressBarProp->setValue(0);
  UpdateSolveState(false);

  emit StartSolve(maxProp);
}
