#include <QPushButton>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QFile>

#include "ColumnEditImage.h"


QWidget* ColumnEditImage::CreateControl(QWidget* parent)
{
  mCtrl = new QPushButton(parent);
  mCtrl->setText("...");
  mCtrl->setFixedSize(mSize);
  mCtrl->setIconSize(mSize);
  mCtrl->setToolTip(tr("Click to change"));

  connect(mCtrl, &QPushButton::clicked, this, &ColumnEditImage::OnPushIcon);
  return mCtrl;
}

bool ColumnEditImage::LoadValue(const QVariant& value)
{
  mData = value.toByteArray();
  QImage image = QImage::fromData(mData);
  QIcon icon(QPixmap::fromImage(image));
  SetIcon(icon);
  return true;
}

bool ColumnEditImage::SaveValue(QVariant& value)
{
  value = mData;
  return true;
}

void ColumnEditImage::SetIcon(const QIcon& icon)
{
  mCtrl->setIcon(icon);
  mCtrl->setText(icon.isNull()? "...": "");
}

void ColumnEditImage::OnPushIcon()
{
  QString filePath = QFileDialog::getOpenFileName(mCtrl, tr("Browse image file"), QApplication::applicationDirPath());
  if (!filePath.isEmpty()) {
    QFile file(filePath);
    if (file.exists()) {
      if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        QImage image = QImage::fromData(data);
        if (!image.isNull()) {
          mData = data;
          QImage image = QImage::fromData(data);
          QIcon icon(QPixmap::fromImage(image));
          SetIcon(icon);
        } else {
          QMessageBox::warning(mCtrl, tr("Browse image file"), QString(tr("File image read fail")));
        }
      } else {
        QMessageBox::warning(mCtrl, tr("Browse image file"), QString(tr("File open fail ('%1')")).arg(file.errorString()));
      }
    }
  }
}


ColumnEditImage::ColumnEditImage(int width, int height)
{
  width = qBound(32, width, 2000);
  mSize.setWidth(width);
  mSize.setHeight(height? height: width);
}
