#include <QTextStream>

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
  *mWriter << '\n';
  return mWriter->status() == QTextStream::Ok;
}

bool CsvWriter::WritePlaneValue(const QByteArray& value)
{
  if (mLineStart) {
    mLineStart = false;
  } else {
    *mWriter << ';';
  }
  *mWriter << value;
  return mWriter->status() == QTextStream::Ok;
}

bool CsvWriter::WriteQuotedValue(const QByteArray& value)
{
  QByteArray newValue = value;
  newValue.replace("\"", "\"\"");
  if (mLineStart) {
    mLineStart = false;
  } else {
    *mWriter << ';';
  }
  *mWriter << '\"' << newValue << '\"';
  return mWriter->status() == QTextStream::Ok;
}


CsvWriter::CsvWriter(QIODevice* _Device, bool _AlwaysQuoted)
  : mDevice(_Device), mAlwaysQuoted(_AlwaysQuoted)
  , mWriter(new QTextStream(_Device)), mLineStart(true)
{
  mWriter->setCodec("UTF-8");
  mWriter->setGenerateByteOrderMark(true);
}

CsvWriter::~CsvWriter()
{
  delete mWriter;
}
