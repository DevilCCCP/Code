#include "CsvWriter.h"



bool CsvWriter::WriteLine(const QByteArray& line)
{
  return WriteValue(line) && WriteEndLine();
}

bool CsvWriter::WriteLine(const QStringList& values)
{
  for (auto itr = values.begin(); itr != values.end(); itr++) {
    const QString& value = *itr;
    if (!WriteValue(value.toUtf8())) {
      return false;
    }
  }
  return WriteEndLine();
}

bool CsvWriter::WriteLine(const QList<QByteArray>& values)
{
  for (auto itr = values.begin(); itr != values.end(); itr++) {
    if (!WriteValue(*itr)) {
      return false;
    }
  }
  return WriteEndLine();
}

bool CsvWriter::WriteValue(const QByteArray& value)
{
  if (mAlwaysQuoted) {
    return WriteQuotedValue(value);
  }

  for (auto itr = value.constBegin(); itr != value.constEnd(); itr++) {
    auto ch = *itr;
    switch (ch) {
    case '"':
    case '\t':
    case ';':
    case '\n':
      return WriteQuotedValue(value);
    }
  }
  return WritePlaneValue(value);
}

bool CsvWriter::WriteEndLine()
{
  mLineStart = true;
  return mDevice->write("\n");
}

bool CsvWriter::WritePlaneValue(const QByteArray& value)
{
  if (mLineStart) {
    mLineStart = false;
    return mDevice->write(value) == value.size();
  } else {
    return mDevice->write(QByteArray(";") + value) == value.size() + 1;
  }
}

bool CsvWriter::WriteQuotedValue(const QByteArray& value)
{
  QByteArray newValue = value;
  newValue.replace("\"", "\"\"");
  if (mLineStart) {
    mLineStart = false;
    return mDevice->write(QByteArray("\"") + newValue + QByteArray("\"")) == newValue.size() + 2;
  } else {
    return mDevice->write(QByteArray(";\"") + newValue + QByteArray("\"")) == newValue.size() + 3;
  }
}


CsvWriter::CsvWriter(QIODevice* _Device, bool _AlwaysQuoted)
  : mDevice(_Device), mAlwaysQuoted(_AlwaysQuoted)
  , mLineStart(true)
{
}
