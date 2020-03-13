#include "DialogGameState.h"
#include "ui_DialogGameState.h"
#include "GameState.h"
#include "Account.h"
#include "Core.h"


DialogGameState::DialogGameState(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogGameState)
{
  ui->setupUi(this);

  ui->labelState->setPixmap(qGameState->BigIcon().pixmap(ui->labelState->size()));
  switch (qGameState->getState()) {
  case GameState::eNoSolve     : ui->labelTytle->setText("Рисунок не собран"); ui->labelTytleDescr->setText(""); break;
  case GameState::eBadSolve    : ui->labelTytle->setText("Рисунок не точен"); ui->labelTytleDescr->setText("попытайтесь найти более подходящую комбинацию"); break;
  case GameState::eUnknownSolve: ui->labelTytle->setText("Цифры совпадают"); ui->labelTytleDescr->setText("точен ли рисунок узнать невозможно, т.к. он был составлен из цифр"); break;
  case GameState::eAiSolve     : ui->labelTytle->setText("Рисунок собран ИИ"); ui->labelTytleDescr->setText(""); break;
  case GameState::eManSolve    : ui->labelTytle->setText("Рисунок собран"); ui->labelTytleDescr->setText(""); break;
  }
  if (qGameState->getState() <= GameState::eBadSolve) {
    ui->commandLinkContinue->setDefault(true);
  } else {
    ui->commandLinkDone->setDefault(true);
  }
  ui->checkBoxAuto->setChecked(!qAccount->getShowGameStateDialog());
  setWindowTitle(qCore->getProgramName());
}

DialogGameState::~DialogGameState()
{
  delete ui;
}


void DialogGameState::on_commandLinkContinue_clicked()
{
  mSwitchType = Account::eWait;
  reject();
}

void DialogGameState::on_commandLinkDone_clicked()
{
  mSwitchType = Account::eDone;
  accept();
}

void DialogGameState::on_commandLinkTooEasy_clicked()
{
  mSwitchType = Account::eTooEasy;
  accept();
}

void DialogGameState::on_commandLinkTooHard_clicked()
{
  mSwitchType = Account::eTooHard;
  accept();
}

void DialogGameState::on_checkBoxAuto_clicked(bool checked)
{
  qAccount->mShowGameStateDialog = !checked;
  if (!qAccount->Save()) {
    qCore->Warning("Сохранение настройки не удалось");
  }
}
