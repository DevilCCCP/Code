#include <QStringList>

#include "Format.h"


const QStringList kBytesSuffixesLatin1 = QStringList() << "B" << "KB" << "MB" << "GB" << "TB";
#ifdef LANG_EN
const QStringList kBytesSuffixes = kBytesSuffixesLatin1;
#else
const QStringList kBytesSuffixes = QStringList() << "Б" << "КБ" << "МБ" << "ГБ" << "ТБ";
#endif
const QStringList kTimeSuffixesLatin1 = QStringList() << "ms" << "s" << "m" << "h" << "d";
#ifdef LANG_EN
const QStringList kTimeSuffixes = kTimeSuffixesLatin1;
#else
const QStringList kTimeSuffixes = QStringList() << "мс" << "с" << "м" << "ч" << "д";
#endif

void FormatBytes(qreal bytes, qreal& value, int& suffix)
{
  if (bytes >= 1.0f * 1024 * 1024 * 1024 * 1024) {
    value = (qreal)bytes / (1.0f * 1024 * 1024 * 1024 * 1024);
    suffix = 4;
  } else if (bytes >= 1024 * 1024 * 1024) {
    value = (qreal)bytes / (1.0f * 1024 * 1024 * 1024);
    suffix = 3;
  } else if (bytes >= 1024 * 1024) {
    value = (qreal)bytes / (1.0f * 1024 * 1024);
    suffix = 2;
  } else if (bytes >= 1024) {
    value = (qreal)bytes / (1.0f * 1024);
    suffix = 1;
  } else {
    value = (qreal)bytes;
    suffix = 0;
  }
}

QString FormatBytes(qreal bytes)
{
  qreal value;
  int suffix;
  FormatBytes(bytes, value, suffix);

  if (value > 100.0) {
    return QString("%1 %2").arg(value, 4, 'f', 0).arg(kBytesSuffixesLatin1[suffix]);
  } else if (value > 10.0) {
    return QString("%1 %2").arg(value, 4, 'f', 1).arg(kBytesSuffixesLatin1[suffix]);
  } else {
    return QString("%1 %2").arg(value, 4, 'f', 2).arg(kBytesSuffixesLatin1[suffix]);
  }
}

QString BytesSuffixToString(int suffix)
{
  return kBytesSuffixes.at(suffix);
}

QString FormatBytesRu(qreal bytes)
{
  qreal value;
  int suffix;
  FormatBytes(bytes, value, suffix);

  if (value > 100.0) {
    return QString("%1 %2").arg(value, 4, 'f', 0).arg(kBytesSuffixes[suffix]);
  } else if (value > 10.0) {
    return QString("%1 %2").arg(value, 4, 'f', 1).arg(kBytesSuffixes[suffix]);
  } else {
    return QString("%1 %2").arg(value, 4, 'f', 2).arg(kBytesSuffixes[suffix]);
  }
}

QString FormatTimeDelta(qint64 ms)
{
  if (ms >= 0) {
    return FormatTime((quint32)ms);
  } else {
    return QString("-") + FormatTime((quint32)(-ms));
  }
}

QString FormatTime(qint64 ms)
{
  if (ms > 2 * 24 * 60 * 60 * 1000) {
    int primeValue = ms / (24 * 60 * 60 * 1000);
    int secondValue = (ms % (24 * 60 * 60 * 1000));
    secondValue = secondValue / (60 * 60 * 1000);
    return QString("%1 d %2 h").arg(primeValue).arg(secondValue);
  } else if (ms > 60 * 60 * 1000) {
    int primeValue = ms / (60 * 60 * 1000);
    int secondValue = (ms % (60 * 60 * 1000));
    secondValue = secondValue / (60 * 1000);
    return QString("%1:%2 h").arg(primeValue).arg(secondValue, 2, 10, QChar('0'));
  } else if (ms > 60 * 1000) {
    int primeValue = ms / (60 * 1000);
    int secondValue = (ms % (60 * 1000));
    secondValue = secondValue / 1000;
    return QString("%1:%2 m").arg(primeValue).arg(secondValue, 2, 10, QChar('0'));
  } else if (ms > 1000) {
    qreal value = (float)ms / (1.0f * 1000);
    if (value > 10) {
      return QString("%1 s").arg(value, 4, 'f', 1);
    } else {
      return QString("%1 s").arg(value, 4, 'f', 2);
    }
  } else {
    return QString("%1 ms").arg(ms);
  }
}

void FormatTime(quint32 ms, qreal& value, int& suffix)
{
  if (ms >= 2 * 24 * 60 * 60 * 1000) {
    value = (qreal)ms / (24 * 60 * 60 * 1000);
    suffix = 4;
  } else if (ms >= 60 * 60 * 1000) {
    value = (qreal)ms / (60 * 60 * 1000);
    suffix = 3;
  } else if (ms >= 60 * 1000) {
    value = (qreal)ms / (60 * 1000);
    suffix = 2;
  } else if (ms >= 1000) {
    value = (qreal)ms / (1000);
    suffix = 1;
  } else {
    value = (qreal)ms;
    suffix = 0;
  }
}

QString FormatTimeDeltaRu(qint64 ms, int prec)
{
  if (ms >= 0) {
    return FormatTimeRu(ms, prec);
  } else {
    return QString("-") + FormatTimeRu(-ms, prec);
  }
}

QString FormatTimeRu(qint64 ms, int prec)
{
  if (ms > 2 * 24 * 60 * 60 * 1000) {
    if (prec <= 1) {
      return QString("%1 д").arg(qRound(ms / (24 * 60 * 60 * 1000.0)));
    }
    int primeValue = ms / (24 * 60 * 60 * 1000);
    int secondValue = (ms % (24 * 60 * 60 * 1000));
    if (prec <= 2) {
      int secondValueHour = qRound(secondValue / (60 * 60 * 1000.0));
      if (secondValueHour >= 24) {
        primeValue++;
        secondValueHour = 0;
      }
      return QString("%1 д %2 ч").arg(primeValue).arg(secondValueHour);
    } else {
      return QString("%1 д %2").arg(primeValue).arg(FormatTimeRu(secondValue, prec - 1));
    }
  } else if (ms > 60 * 60 * 1000) {
    if (prec <= 1) {
      return QString("%1 ч").arg(qRound(ms / (60 * 60 * 1000.0)));
    }
    int primeValue = ms / (60 * 60 * 1000);
    int secondValue = (ms % (60 * 60 * 1000));
    if (prec <= 2) {
      int secondValueMin = qRound(secondValue / (60 * 1000.0));
      if (secondValueMin == 60) {
        primeValue++;
        secondValueMin = 0;
      }
      return QString("%1:%2 ч").arg(primeValue).arg(secondValueMin, 2, 10, QChar('0'));
    } else if (prec <= 3) {
      int primeValue2 = secondValue / (60 * 1000);
      int secondValue2 = qRound(secondValue / (60 * 1000.0));
      if (secondValue2 == 60) {
        primeValue2++;
        secondValue2 = 0;
        if (primeValue2 == 60) {
          primeValue++;
          primeValue2 = 0;
        }
      }
      return QString("%1:%2:%3").arg(primeValue).arg(primeValue2, 2, 10, QChar('0')).arg(secondValue2, 2, 10, QChar('0'));
    } else {
      int primeValue2 = secondValue / (60 * 1000);
      int secondValue2 = (secondValue % (60 * 1000));
      int primeValue3 = secondValue2 / (1000);
      int primeValue4 = secondValue2 % (1000);
      return QString("%1:%2:%3.%4").arg(primeValue).arg(primeValue2, 2, 10, QChar('0'))
          .arg(primeValue3, 2, 10, QChar('0')).arg(primeValue4, 3, 10, QChar('0'));
    }
  } else if (ms > 60 * 1000) {
    if (prec <= 1) {
      int primeValueMin = qRound(ms / (60 * 1000.0));
      if (primeValueMin == 60) {
        return QString("1 ч");
      }
      return QString("%1 м").arg(primeValueMin);
    }
    int primeValue = ms / (60 * 1000);
    int secondValue = (ms % (60 * 1000));
    if (prec <= 2) {
      int secondValueSec = qRound(secondValue / 1000.0);
      if (secondValueSec == 60) {
        primeValue++;
        secondValueSec = 0;
        if (primeValue == 60) {
          return QString("1:00 ч");
        }
      }
      return QString("%1:%2 м").arg(primeValue).arg(secondValueSec, 2, 10, QChar('0'));
    } else {
      int primeValue2 = secondValue / (1000);
      int secondValueMs = qRound(secondValue / (1000.0));
      if (secondValueMs == 1000) {
        primeValue2++;
        secondValueMs = 0;
        if (primeValue2 == 60) {
          primeValue++;
          primeValue2 = 0;
          if (primeValue == 60) {
            return QString("1:00:00 ч");
          }
        }
      }
      return QString("%1:%2.%3").arg(primeValue).arg(primeValue2, 2, 10, QChar('0')).arg(secondValueMs, 3, 10, QChar('0'));
    }
  } else if (ms > 1000) {
    if (prec <= 1) {
      int primeSec = qRound(ms / (1000.0));
      if (primeSec == 60) {
        return QString("1:00 м");
      }
      return QString("%1 с").arg(primeSec);
    }
    qreal value = (float)ms / (1.0f * 1000);
    if (value > 10) {
      return QString("%1 с").arg(value, 4, 'f', 1);
    } else {
      return QString("%1 с").arg(value, 4, 'f', 2);
    }
  } else {
    return QString("%1 мс").arg(ms);
  }
}

