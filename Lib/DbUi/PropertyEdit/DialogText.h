#pragma once

#include <QDialog>


namespace Ui {
class DialogText;
}

class DialogText: public QDialog
{
  Ui::DialogText *ui;

  Q_OBJECT

public:
  explicit DialogText(QWidget* parent = 0);
  ~DialogText();

public:
  QString Text();
  void SetText(const QString& text);

signals:
  void Accepted();
  void Rejected();

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
};
