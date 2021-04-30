#include <Lib/UpdaterCore/UpdateSettings.h>
#include <Lib/UpdaterCore/InstallerSimple.h>

#include "FormUpdateSettings.h"
#include "ui_FormUpdateSettings.h"


FormUpdateSettings::FormUpdateSettings(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormUpdateSettings)
  , mInstaller(nullptr)
{
  ui->setupUi(this);

  UpdateControls();
}

FormUpdateSettings::~FormUpdateSettings()
{
  delete ui;
}


QString FormUpdateSettings::StateToString(int state)
{
  switch ((InstallState)state) {
  case eNotChecked  : return tr("Not checked");
  case eChecking    : return tr("Checking");
  case eCheckedFalse: return tr("No update");
  case eCheckedTrue : return tr("Has update");
  case eCheckError  : return tr("Check error");
  case eLoading     : return tr("Loading");
  case eLoadTrue    : return tr("Loaded");
  case eLoadError   : return tr("Loading error");
  case eInstalling  : return tr("Installing");
  }
  return QString();
}

void FormUpdateSettings::SetInstaller(InstallerSimple* _Installer)
{
  mInstaller = _Installer;

  connect(mInstaller, &InstallerSimple::StateChanged, this, &FormUpdateSettings::OnUpdateStateChanged);
}

void FormUpdateSettings::ReadSettings(QSettings* settings)
{
  UpdateCheck updateCheck = (UpdateCheck)settings->value("UpdateCheck", eAutoOnStart).toInt();
  switch (updateCheck) {
  case eManualCheck: ui->radioButtonCheckManual->setChecked(true); break;
  case eAutoOnStart: ui->radioButtonCheckStart->setChecked(true); break;
  }

  UpdateInstall updateInstall = (UpdateInstall)settings->value("UpdateInstall", eAskInstall).toInt();
  switch (updateInstall) {
  case eManualInstall: ui->radioButtonInstallManual->setChecked(true); break;
  case eAskInstall   : ui->radioButtonInstallAsk->setChecked(true); break;
  case eInstallOnExit: ui->radioButtonInstallOnExit->setChecked(true); break;
  }

  UpdateControls();
}

void FormUpdateSettings::WriteSettings(QSettings* settings)
{
  UpdateCheck updateCheck = eAutoOnStart;
  if (ui->radioButtonCheckManual->isChecked()) {
    updateCheck = eManualCheck;
  } else if (ui->radioButtonCheckStart->isChecked()) {
    updateCheck = eAutoOnStart;
  }
  settings->setValue("UpdateCheck", (int)updateCheck);

  UpdateInstall updateInstall = eAskInstall;
  if (ui->radioButtonInstallManual->isChecked()) {
    updateInstall = eManualInstall;
  } else if (ui->radioButtonInstallAsk->isChecked()) {
    updateInstall = eAskInstall;
  } else if (ui->radioButtonInstallOnExit->isChecked()) {
    updateInstall = eInstallOnExit;
  }
  settings->setValue("UpdateInstall", (int)updateInstall);

  if (mInstaller) {
    mInstaller->ReloadSettings(settings);
  }
}

void FormUpdateSettings::UpdateControls()
{
  ui->widgetState->setVisible(mInstaller);
  if (mInstaller) {
    ui->lineEditState->setText(StateToString(mInstaller->State()));
    ui->pushButtonCheck->setEnabled(mInstaller->CanCheck());
    ui->pushButtonInstall->setEnabled(mInstaller->CanLoadAndInstall());
  }
}

void FormUpdateSettings::OnUpdateStateChanged(int state)
{
  ui->lineEditState->setText(StateToString(state));

  UpdateControls();
}

void FormUpdateSettings::on_pushButtonCheck_clicked()
{
  if (mInstaller) {
    mInstaller->StartCheck();
  }
}

void FormUpdateSettings::on_pushButtonInstall_clicked()
{
  if (mInstaller) {
    mInstaller->StartLoadAndInstall();
  }
}
