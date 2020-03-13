#pragma once

#include <QIODevice>
#include <QStringList>

#include <Lib/Include/Common.h>


class CsvReader
{
  QIODevice*   mDevice;

public:
  bool AtEnd() { return mDevice->atEnd(); }
  bool ReadLine(QStringList& line);

public:
  CsvReader(QIODevice* _Device);
};

