#include "StringUtils.h"


QString StringEscape(const QString& source)
{
  QString dest;
  for (auto itr = source.constBegin(); itr != source.constEnd(); itr++) {
    const QChar& chr = *itr;
    char ch = chr.toLatin1();
    switch (ch) {
    case '\\': dest.append("\\\\"); break;
    case '\n': dest.append("\\n"); break;
    default: dest.append(chr); break;
    }
  }
  return dest;
}

QString StringUnescape(const QString& source)
{
  QString dest;
  for (auto itr = source.constBegin(); itr != source.constEnd(); itr++) {
    const QChar& chr = *itr;
    char ch = chr.toLatin1();
    switch (ch) {
    case '\\':
      if (++itr != source.constEnd()) {
        const QChar& chr = *itr;
        char ch = chr.toLatin1();
        switch (ch) {
        case '\\': dest.append('\\'); break;
        case 'n': dest.append('\n'); break;
        default: break;
        }
      }
      break;
    default: dest.append(chr); break;
    }
  }
  return dest;
}
