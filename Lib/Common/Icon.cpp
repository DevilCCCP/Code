#include "Icon.h"


QIcon IconFromImage(const QString& main, const QString& ext, bool top, bool left)
{
  QPixmap mainImage(main);
  QPixmap extImage(ext);

  QPainter painter(&mainImage);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.drawPixmap(left? 0: mainImage.width()/2, top? 0: mainImage.height()/2, mainImage.width()/2, mainImage.height()/2, extImage);

  return QIcon(mainImage);
}

QIcon SmallIcon(const QString& icon, int width, int height)
{
  QPixmap mainImage(16, 16);
  QPixmap image(icon);

  mainImage.fill(Qt::transparent);
  QPainter painter(&mainImage);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.drawPixmap(0, 0, width, height, image);

  return QIcon(mainImage);
}

QIcon GrayIcon(const QIcon& baseIcon, int iconSize)
{
  int width = iconSize;
  int height = iconSize;
  if (iconSize <= 0) {
    if (!baseIcon.availableSizes().isEmpty()) {
      auto sz = baseIcon.availableSizes();
      width = baseIcon.availableSizes().first().width();
      height = baseIcon.availableSizes().first().height();
    } else {
      width = height = kSmallIconSize;
    }
  }

  QPixmap pixmapIcon(width, height);
  pixmapIcon.fill(QColor(0, 0, 0, 0));
  QPainter painter(&pixmapIcon);
  baseIcon.paint(&painter, 0, 0, width, height, Qt::AlignCenter, QIcon::Disabled);
  return QIcon(pixmapIcon);
}
