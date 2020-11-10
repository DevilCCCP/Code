#include "FormEarth.h"
#include "ui_FormEarth.h"

FormEarth::FormEarth(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FormEarth)
{
  ui->setupUi(this);
}

FormEarth::~FormEarth()
{
  delete ui;
}
