#pragma once

#include <QWidget>
#include <QSettings>


namespace Ui {
class FormUpdateSettings;
}

class InstallerSimple;

class FormUpdateSettings: public QWidget
{
  Ui::FormUpdateSettings* ui;

  InstallerSimple*        mInstaller;

  Q_OBJECT

public:
  explicit FormUpdateSettings(QWidget* parent = 0);
  ~FormUpdateSettings();

public:
  QString StateToString(int state);
  void SetInstaller(InstallerSimple* _Installer);

  void ReadSettings(QSettings* settings);
  void WriteSettings(QSettings* settings);

private:
  void UpdateControls();

private slots:
  void OnUpdateStateChanged(int state);

private slots:
  void on_pushButtonCheck_clicked();
  void on_pushButtonInstall_clicked();
};
