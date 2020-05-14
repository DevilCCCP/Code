#pragma once

#include <QDialog>


namespace Ui {
class DialogSolve;
}

class Account;
class Puzzle;

class DialogSolve: public QDialog
{
  Ui::DialogSolve* ui;

  Account*         mAccount;
  Puzzle*          mPuzzle;

  Q_OBJECT

public:
  explicit DialogSolve(QWidget* parent = 0);
  ~DialogSolve();

public:
  void Init(Account* account, Puzzle* puzzle);

private:
  void UpdateSolveState(bool ready);

signals:
  void StartSolve(int maxProp);
  void StopSolve();

public slots:
  void OnSolveChanged(int solved, int prop);
  void OnSolveDone(bool result);

private slots:
  void on_pushButtonCancel_clicked();
  void on_checkBoxAuto_toggled(bool checked);
  void on_pushButtonStart_clicked();
};
