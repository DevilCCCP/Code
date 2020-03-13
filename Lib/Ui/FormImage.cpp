#include <QRegExp>

#include "FormImage.h"
#include "ui_FormImage.h"


FormImage::FormImage(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormImage)
{
  ui->setupUi(this);
  ui->toolButtonShrink->setDefaultAction(ui->actionShrink);
  ui->actionShrink->setChecked(true);
  SetShrink(true);
}

FormImage::~FormImage()
{
  delete ui;
}


void FormImage::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);

  DrawImage();
}

void FormImage::SetImage(const QImage& image)
{
  mImage = image;
  SetShrink(ui->actionShrink->isChecked());
  ui->labelDimentions->setText(QString("width: %1; height: %2").arg(mImage.width()).arg(mImage.height()));
}

void FormImage::SetShrink(bool shrink)
{
  mShrink = shrink;
  DrawImage();
}

void FormImage::DrawImage()
{
  int maxWidth  = ui->scrollAreaMain->width() - 4;
  int maxHeight = ui->scrollAreaMain->height() - 4;
  if (mShrink && (maxWidth < mImage.width() || maxHeight < mImage.height())) {
    QSize clientSize(maxWidth, maxHeight);
    ui->labelMainImage->setPixmap(QPixmap::fromImage(mImage).scaled(clientSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  } else {
    ui->labelMainImage->setPixmap(QPixmap::fromImage(mImage));
  }
}

void FormImage::on_actionShrink_triggered(bool checked)
{
  SetShrink(checked);
}
