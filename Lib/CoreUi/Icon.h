#pragma once

#include <QIcon>
#include <QPixmap>
#include <QPainter>


const int kSmallIconSize = 16;

QIcon IconFromImage(const QString& main, const QString& ext, bool top = false, bool left = false);

QIcon SmallIcon(const QString& icon, int width = kSmallIconSize, int height = kSmallIconSize);

QIcon GrayIcon(const QIcon& baseIcon, int iconSize = 0);
