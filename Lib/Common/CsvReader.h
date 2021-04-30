#pragma once

#include <QIODevice>
#include <QStringList>

#include <Lib/Include/Common.h>


class QTextStream;

class CsvReader
{
  QIODevice*   mDevice;

  QTextStream* mReader;

public:
  bool AtEnd() { return mDevice->atEnd(); }
  bool ReadLine(QList<QByteArray>& line);
  bool ReadLine(QStringList& line);

public:
  CsvReader(QIODevice* _Device);
};

