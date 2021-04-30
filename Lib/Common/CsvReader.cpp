#include <QTextStream>

#include "CsvReader.h"


bool inline ParseSimple(QByteArray::const_iterator& itr, QByteArray::const_iterator& itre, QByteArray& value)
{
  for (; itr != itre; itr++) {
    auto ch = *itr;
    switch (ch) {
    case '\r': // ignore windows extra EOL
      break;
    case '\n':
    case ';':
    case '\t':
      return true;

    default:
      value.append(ch);
      break;
    }
  }
  return true;
}

bool inline ParseSimple(QString::const_iterator& itr, QString::const_iterator& itre, QString& value)
{
  for (; itr != itre; itr++) {
    const QChar& ch = *itr;
    switch (ch.toLatin1()) {
    case '\r': // ignore windows extra EOL
      break;
    case '\n':
    case ';':
    case '\t':
      return true;

    default:
      value.append(ch);
      break;
    }
  }
  return true;
}

bool inline ParseQuoted(QByteArray::const_iterator& itr, QByteArray::const_iterator& itre, QByteArray& value)
{
  for (; itr != itre; itr++) {
    auto ch = *itr;
    switch (ch) {
    case '"':
      for (itr++; itr != itre && *itr == '\r'; itr++) {
      }
      if (itr != itre) {
        ch = *itr;
        switch (ch) {
        case '\n':
        case ';':
        case '\t':
          return true;

        case '"': // 2x qoute = qoute
          value.append('"');
          break;

        default: // undefined behavior
          return ParseSimple(itr, itre, value);
          break;
        }
      }
      break;

    case '\n':
      value.append(ch);
      return false;

    default:
      value.append(ch);
      break;
    }
  }
  return true;
}

bool inline ParseQuoted(QString::const_iterator& itr, QString::const_iterator& itre, QString& value)
{
  for (; itr != itre; itr++) {
    const QChar& ch = *itr;
    switch (ch.toLatin1()) {
    case '"':
      for (itr++; itr != itre && *itr == '\r'; itr++) {
      }
      if (itr != itre) {
        const QChar& ch = *itr;
        switch (ch.toLatin1()) {
        case '\n':
        case ';':
        case '\t':
          return true;

        case '"': // 2x qoute = qoute
          value.append('"');
          break;

        default: // undefined behavior
          return ParseSimple(itr, itre, value);
          break;
        }
      }
      break;

    case '\n':
      value.append(ch);
      return false;

    default:
      value.append(ch);
      break;
    }
  }
  return true;
}

bool CsvReader::ReadLine(QList<QByteArray>& line)
{
  line.clear();
  QByteArray data = mDevice->readLine();
  if (data.isEmpty()) {
    return AtEnd();
  }

  QByteArray value;
  // 'end of item' parse state
  auto itre = data.constEnd();
  for (auto itr = data.constBegin(); itr != itre; ) {
    auto ch = *itr;
    switch (ch) {
    case '\r': // ignore windows extra EOL
      itr++;
      break;

    case '\n': // 'end of line' parse state
      line << value;
      return true;

    case ';': // 'end of value' (empty value) parse state
    case '\t':
      line << value;
      value.clear();
      itr++;
      break;

    case '"':
      itr++;
      while (!ParseQuoted(itr, itre, value)) {
        data = mDevice->readLine();
        itr = data.constBegin();
        itre = data.constEnd();
      }
      break;

    default:
      ParseSimple(itr, itre, value);
      break;
    }
  }
  line << value;
  return true;
}

bool CsvReader::ReadLine(QStringList& line)
{
  line.clear();
  QString text = mReader->readLine();
  if (text.isEmpty()) {
    return AtEnd();
  }

  QString value;
  // 'end of item' parse state
  auto itre = text.constEnd();
  for (auto itr = text.constBegin(); itr != itre; ) {
    auto ch = *itr;
    switch (ch.toLatin1()) {
    case '\r': // ignore windows extra EOL
      itr++;
      break;

    case '\n': // 'end of line' parse state
      line << value;
      return true;

    case ';': // 'end of value' (empty value) parse state
    case '\t':
      line << value;
      value.clear();
      itr++;
      break;

    case '"':
      itr++;
      while (!ParseQuoted(itr, itre, value)) {
        text = mReader->readLine();
        itr = text.constBegin();
        itre = text.constEnd();
      }
      break;

    default:
      ParseSimple(itr, itre, value);
      break;
    }
  }
  line << value;
  return true;
}


CsvReader::CsvReader(QIODevice* _Device)
  : mDevice(_Device)
  , mReader(new QTextStream(_Device))
{
  mReader->setCodec("UTF-8");
}

