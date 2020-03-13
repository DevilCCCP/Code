#pragma once

#include <QString>


class CoreInfo {
public:
  /*new */virtual void Info(const QString& text) = 0;
  /*new */virtual void Warning(const QString& text) = 0;
  /*new */virtual void Error(const QString& text) = 0;

public:
  CoreInfo() {}
  /*new */virtual ~CoreInfo() {}
};
