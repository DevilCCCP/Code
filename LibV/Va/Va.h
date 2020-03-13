#pragma once

#include <QMap>

#include <LibV/Include/Rect.h>
#include <LibV/Include/Frame.h>


#pragma pack(push, 2)
struct Object
{
  qint64    Id;
  Rectangle Dimention;
  int       Color; /* int ind = Color / 1000; int value = qMin(100, Color % (1000*ind)); ind: 0 - White, 1 - Green, 2 - Red, 3 - Blue (gObjectColors)*/
};
#pragma pack(pop)

struct Detector {
  int     Id;
  QString Name;

  QMap<QString, QString> Settings;

  /*new */virtual void Hit(const qint64& time, const char* type) { return Hit(time, type, 1); }
  /*new */virtual void Hit(const qint64& time, const char* type, qreal value) = 0;
  /*new */virtual void Hit(const qint64& time, const char* type, qreal value, const QString& info) { Q_UNUSED(info); return Hit(time, type, value); }
  /*new */virtual void Hit(const qint64& time, const char* type, const QByteArray& img) { return Hit(time, type, 1, img); }
  /*new */virtual void Hit(const qint64& time, const char* type, qreal value, const QByteArray& img) { Q_UNUSED(img); return Hit(time, type, value); }
  /*new */virtual void Hit(const qint64& time, const char* type, qreal value, const QString& info, const QByteArray& img) { Q_UNUSED(info); return Hit(time, type, value, img); }
  /*new */virtual void Stat(const qint64& time, const char* type, qreal value, int periodMs) = 0;

  virtual ~Detector() { }
};
