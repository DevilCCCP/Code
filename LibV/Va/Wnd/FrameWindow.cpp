#include "FrameWindow.h"
#include "ui_FrameWindow.h"


FrameWindow::FrameWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::FrameWindow)
{
  ui->setupUi(this);
}

FrameWindow::~FrameWindow()
{
  delete ui;
}


void FrameWindow::SetImage(const QImage& image)
{
  ui->formImage->SetBackImage(image);
}
