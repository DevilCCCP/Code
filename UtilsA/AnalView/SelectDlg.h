#ifndef SELECTDLG_H
#define SELECTDLG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QList>

namespace Ui {
class SelectDlg;
}

class SelectDlg : public QDialog
{

public:
  void SetLeft(const QList<QString>& _Left);
  void GetRight(QList<QString>& _Right);

  Q_OBJECT

public:
  explicit SelectDlg(QWidget *parent = 0);
  ~SelectDlg();

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();
  void on_listWidgetLeft_itemClicked(QListWidgetItem *item);

  void on_listWidgetRight_itemClicked(QListWidgetItem *item);

private:
  Ui::SelectDlg *ui;
};

#endif // SELECTDLG_H
