#include "DialogFrame.h"
#include "ui_DialogFrame.h"


DialogFrame::DialogFrame(QWidget* parent)
  : QDialog(parent), ui(new Ui::DialogFrame)
{
  ui->setupUi(this);
}

DialogFrame::~DialogFrame()
{
  delete ui;
}


void DialogFrame::SetImage(const QImage& image)
{
  ui->formImage->SetBackImage(image);
}
