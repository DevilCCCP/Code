#include "FormMapPreview.h"
#include "ui_FormMapPreview.h"

FormMapPreview::FormMapPreview(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FormMapPreview)
{
  ui->setupUi(this);
}

FormMapPreview::~FormMapPreview()
{
  delete ui;
}
