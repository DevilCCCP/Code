#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include <Lib/Include/Common.h>


DefineClassS(Core);

class Core: public QObject
{
  static Core*               mSelf;

  PROPERTY_GET(QString,      ProgramName)

  PROPERTY_GET(QString,      Version)

  Q_OBJECT
  ;
public:
  static Core* Instance() { return mSelf; }

public:
  bool InitVersion();

private:

signals:

public:
  Core();
};

#define qCore (Core::Instance())
