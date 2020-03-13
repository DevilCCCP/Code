#include "SelectDlg.h"
#include "ui_SelectDlg.h"


void SelectDlg::SetLeft(const QList<QString> &_Left)
{
  for (auto itr = _Left.begin(); itr != _Left.end(); itr++) {
    ui->listWidgetLeft->addItem(*itr);
  }
}

void SelectDlg::GetRight(QList<QString> &_Right)
{
  _Right.clear();
  while (QListWidgetItem* item = ui->listWidgetRight->takeItem(0)) {
    _Right.append(item->text());
  }
}


SelectDlg::SelectDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SelectDlg)
{
  ui->setupUi(this);
}

SelectDlg::~SelectDlg()
{
  delete ui;
}

void SelectDlg::on_buttonBox_accepted()
{
  emit accept();
}

void SelectDlg::on_buttonBox_rejected()
{
  emit reject();
}

void SelectDlg::on_listWidgetLeft_itemClicked(QListWidgetItem *item)
{
  ui->listWidgetLeft->removeItemWidget(item);
  ui->listWidgetRight->addItem(item->text());
  delete item;
}

void SelectDlg::on_listWidgetRight_itemClicked(QListWidgetItem *item)
{
  ui->listWidgetRight->removeItemWidget(item);
  ui->listWidgetLeft->addItem(item->text());
  delete item;
}
