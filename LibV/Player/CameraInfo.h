#pragma once

#include <QString>

#include <Lib/Common/Uri.h>


struct CameraInfo {
  int     Id;
  QString Name;
  Uri     VideoUri;
  qint64  Timestamp;

  CameraInfo(): Timestamp(0) { }
};
