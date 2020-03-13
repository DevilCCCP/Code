#pragma once

#include <QDialog>


namespace Ui {
class DialogLogin;
}
class QStringListModel;

class DialogLogin: public QDialog
{
  Ui::DialogLogin*  ui;

  QStringListModel* mUserList;

  Q_OBJECT

public:
  explicit DialogLogin(QWidget* parent = 0);
  ~DialogLogin();

public:
  QString Name() const;

private slots:
  void on_comboBoxName_editTextChanged(const QString& text);

private:
};
