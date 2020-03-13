#pragma once

#include <QDialog>


namespace Ui {
class DialogSettings;
}
class Account;

class DialogSettings: public QDialog
{
  Ui::DialogSettings* ui;

  Q_OBJECT

public:
  explicit DialogSettings(QWidget* parent = 0);
  ~DialogSettings();

public:
  void Load(Account* account);
  void Save(Account* account);

private:
  void AutoInfo();

private slots:
  void on_toolButtonInfoStackLimit_clicked();
  void on_toolButtonLevelProfile_clicked();
  void on_toolButtonHighlight_clicked();
  void on_toolButtonCompactDigits_clicked();
  void on_toolButtonDigits_clicked();
  void on_toolButtonInfoAutoSave_clicked();
  void on_radioButtonCompactDigitsOn_toggled(bool checked);
  void on_toolButtonPreview_clicked();
  void on_toolButtonCalcWindow_clicked();
  void on_comboBoxLevelProfile_currentIndexChanged(int index);
};
