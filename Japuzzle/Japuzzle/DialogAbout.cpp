#include "DialogAbout.h"
#include "ui_DialogAbout.h"
#include "Core.h"


DialogAbout::DialogAbout(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogAbout)
{
  ui->setupUi(this);

  setWindowTitle(qCore->getProgramName());
  ui->labelVersion->setText(QString("Версия %1").arg(qCore->getVersion()));
}

DialogAbout::~DialogAbout()
{
  delete ui;
}
