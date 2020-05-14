#include <QMessageBox>
#include <QStringListModel>

#include "DialogSettings.h"
#include "ui_DialogSettings.h"
#include "Core.h"
#include "Account.h"


enum ProfileLevel {
  eLevelEasy,
  eLevelNormal,
  eLevelHard,
  eLevelCustom
};

const QStringList kProfileLevelList = QStringList() << "Лёгкий" << "Средний" << "Тяжёлый" << "Пользовательский";

DialogSettings::DialogSettings(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogSettings)
{
  ui->setupUi(this);

  ui->formSectionStyle->SetWidget(ui->widgetStyle, "Оформление");
  ui->formSectionLevel->SetWidget(ui->widgetLevel, "Уровень сложности");
  ui->formSectionAdvanced->SetWidget(ui->widgetAdvanced, "Дополнительно (для продвинутых)");
  ui->formSectionAdvanced->SetShow(false);

  ui->comboBoxLevelProfile->setModel(new QStringListModel(kProfileLevelList, this));
  on_comboBoxLevelProfile_currentIndexChanged(0);
}

DialogSettings::~DialogSettings()
{
  delete ui;
}


void DialogSettings::Load(Account* account)
{
  switch(account->getPreviewSize()) {
  case 4 : ui->radioButtonPreview4->setChecked(true); break;
  case 2 : ui->radioButtonPreview2->setChecked(true); break;
  case 1 :
  default: ui->radioButtonPreview1->setChecked(true); break;
  }

  switch (account->getDigitStyle()) {
  case Account::eDigitManual: ui->radioButtonDigitsManual->setChecked(true); break;
  case Account::eDigitAuto  : ui->radioButtonDigitsAuto->setChecked(true); break;
  case Account::eDigitSmart : ui->radioButtonDigitsSmart->setChecked(true); break;
  }
  if (account->getDigitHighlight()) {
    ui->radioButtonHighlightOn->setChecked(true);
  } else {
    ui->radioButtonHighlightOff->setChecked(true);
  }
  if (int count = account->getCompactDigits()) {
    ui->radioButtonCompactDigitsOn->setChecked(true);
    ui->spinBoxCompactDigits->setValue(count);
    ui->spinBoxCompactDigits->setEnabled(true);
  } else {
    ui->radioButtonCompactDigitsOff->setChecked(true);
    ui->spinBoxCompactDigits->setEnabled(false);
  }
  if (account->mCalcWindow == Account::eCalcWindowSmart) {
    ui->radioButtonCalcWindowSmart->setChecked(true);
  } else if (account->mCalcWindow == Account::eCalcWindowSimple) {
    ui->radioButtonCalcWindowSimple->setChecked(true);
  } else {
    ui->radioButtonCalcWindowNo->setChecked(true);
  }
  if (account->getAutoCalcStars()) {
    ui->radioButtonCalcStarsAuto->setChecked(true);
  } else {
    ui->radioButtonCalcStarsManual->setChecked(true);
  }

  ui->spinBoxUndoLimit->setValue(account->getUndoStackLimit());
  ui->doubleSpinBoxAutoSave->setValue(account->getAutoSavePeriod() / 60000.0);
}

void DialogSettings::Save(Account* account)
{
  if (ui->radioButtonPreview1->isChecked()) {
    account->mPreviewSize = 1;
  } else if (ui->radioButtonPreview2->isChecked()) {
    account->mPreviewSize = 2;
  } else if (ui->radioButtonPreview4->isChecked()) {
    account->mPreviewSize = 4;
  } else {
    account->mPreviewSize = 1;
    Q_ASSERT(0);
  }
  if (ui->radioButtonDigitsManual->isChecked()) {
    account->mDigitStyle = Account::eDigitManual;
  } else if (ui->radioButtonDigitsAuto->isChecked()) {
    account->mDigitStyle = Account::eDigitAuto;
  } else if (ui->radioButtonDigitsSmart->isChecked()) {
    account->mDigitStyle = Account::eDigitSmart;
  } else {
    account->mDigitStyle = Account::eDigitSmart;
    Q_ASSERT(0);
  }
  if (ui->radioButtonHighlightOn->isChecked()) {
    account->mDigitHighlight = true;
  } else {
    account->mDigitHighlight = false;
  }
  if (ui->radioButtonCompactDigitsOn->isChecked()) {
    account->mCompactDigits = ui->spinBoxCompactDigits->value();
  } else {
    account->mCompactDigits = 0;
  }
  if (ui->radioButtonCalcWindowSmart->isChecked()) {
    account->mCalcWindow = Account::eCalcWindowSmart;
  } else if (ui->radioButtonCalcWindowSimple->isChecked()) {
    account->mCalcWindow = Account::eCalcWindowSimple;
  } else {
    account->mCalcWindow = Account::eCalcWindowNone;
  }
  if (ui->radioButtonCalcStarsAuto->isChecked()) {
    account->mAutoCalcStars = true;
  } else {
    account->mAutoCalcStars = false;
  }

  account->mUndoStackLimit = ui->spinBoxUndoLimit->value();
  if (!ui->checkBoxAutoSaveDisable->isChecked()) {
    account->mAutoSavePeriod = (int)(ui->doubleSpinBoxAutoSave->value() * 60000.0);
  } else {
    account->mAutoSavePeriod = 0;
  }
}

void DialogSettings::AutoInfo()
{
  QWidget* w = qobject_cast<QWidget*>(sender());
  if (w) {
    QMessageBox::information(this, qCore->getProgramName(), w->toolTip());
  }
}

void DialogSettings::on_toolButtonInfoStackLimit_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonLevelProfile_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonHighlight_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonCompactDigits_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonDigits_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonInfoAutoSave_clicked()
{
  AutoInfo();
}

void DialogSettings::on_radioButtonCompactDigitsOn_toggled(bool checked)
{
  ui->spinBoxCompactDigits->setEnabled(checked);
}

void DialogSettings::on_toolButtonPreview_clicked()
{
  AutoInfo();
}

void DialogSettings::on_toolButtonCalcWindow_clicked()
{
  AutoInfo();
}

void DialogSettings::on_comboBoxLevelProfile_currentIndexChanged(int index)
{
  switch ((ProfileLevel)index) {
  case eLevelEasy:
    ui->radioButtonHighlightOn->setChecked(true);
    ui->radioButtonCompactDigitsOn->setChecked(true);
    ui->spinBoxCompactDigits->setValue(4);
    ui->radioButtonDigitsSmart->setChecked(true);
    ui->radioButtonCalcWindowSmart->setChecked(true);
    break;

  case eLevelNormal:
    ui->radioButtonHighlightOn->setChecked(true);
    ui->radioButtonCompactDigitsOn->setChecked(true);
    ui->spinBoxCompactDigits->setValue(4);
    ui->radioButtonDigitsAuto->setChecked(true);
    ui->radioButtonCalcWindowSimple->setChecked(true);
    break;

  case eLevelHard:
    ui->radioButtonHighlightOff->setChecked(true);
    ui->radioButtonCompactDigitsOff->setChecked(true);
    ui->radioButtonDigitsManual->setChecked(true);
    ui->radioButtonCalcWindowNo->setChecked(true);
    break;

  case eLevelCustom:
    break;
  }
}

void DialogSettings::on_toolButtonCalcStars_clicked()
{
  AutoInfo();
}
