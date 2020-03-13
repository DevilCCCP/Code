#pragma once

#include <QDialog>

#include <Lib/Include/Common.h>


namespace Ui {
class DialogGameState;
}

class DialogGameState: public QDialog
{
  Ui::DialogGameState* ui;

  PROPERTY_GET(int,    SwitchType)

  Q_OBJECT

public:
  explicit DialogGameState(QWidget* parent = 0);
  ~DialogGameState();

private slots:
  void on_commandLinkContinue_clicked();
  void on_commandLinkDone_clicked();
  void on_commandLinkTooEasy_clicked();
  void on_commandLinkTooHard_clicked();
  void on_checkBoxAuto_clicked(bool checked);
};
