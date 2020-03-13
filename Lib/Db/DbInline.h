#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>

#include <Lib/Include/Common.h>


inline QString ToSql(const QString& text)
{
  if (text.indexOf('\'') >= 0) {
    QString text_ = text;
    return QString("'") + text_.replace(QChar('\''), QString("''")) + QString("'");
  } else {
    return QString("'") + text + QString("'");
  }
}

inline QString ToSql(const QDateTime& datetime)
{
  return QString("'%1Z'").arg(datetime.toUTC().toString("yyyy-MM-dd hh:mm:ss.zzz"));
}

inline QString ToSql(const qint64& id)
{
  if (id) {
    return QString("=%1").arg(id);
  } else {
    return QStringLiteral(" IS NULL");
  }
}

inline QString ToSql(const int& id)
{
  if (id) {
    return QString("=%1").arg(id);
  } else {
    return QStringLiteral(" IS NULL");
  }
}

inline QStringList ParseRowSimple(const QString& row)
{
  QStringList list;
  int length = row.length() - 1;
  if (length >= 2 && row[0] == '(' && row[length] == ')') {
    int start = 1;
    for (int i = 1; i < length; i++) {
      if (row[i] == ',') {
        QString value = row.mid(start, i - start);
        list.append(value);
        start = i + 1;
      }
    }
    list.append(row.mid(start, length - start));
  }

  return list;
}
