#pragma once

#include <QIODevice>
#include <QStringList>

#include <Lib/Include/Common.h>


class CsvWriter
{
  QIODevice* mDevice;
  bool       mAlwaysQuoted;

  bool       mLineStart;

public:
  QIODevice* Device() const { return mDevice; }

public:
  bool WriteLine(const QByteArray& line);
  bool WriteLine(const QStringList& values);
  bool WriteLine(const QList<QByteArray>& values);

  bool WriteValue(const QByteArray& value);
  bool WriteEndLine();

private:
  bool WritePlaneValue(const QByteArray& value);
  bool WriteQuotedValue(const QByteArray& value);

public:
  CsvWriter(QIODevice* _Device, bool _AlwaysQuoted = false);
};
