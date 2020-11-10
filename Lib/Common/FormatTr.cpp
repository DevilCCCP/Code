#include <QTranslator>
#include <QCoreApplication>

#include "FormatTr.h"


FormatTr* FormatTr::mSelf = nullptr;

QString FormatTr::FormatTimeDeltaTr(qint64 ms, int prec)
{
  if (ms >= 0) {
    return FormatTimeTr(ms, prec);
  } else {
    return QString("-") + FormatTimeTr(-ms, prec);
  }
}

QString FormatTr::FormatTimeTr(qint64 ms, int prec)
{
  if (ms > 2 * 24 * 60 * 60 * 1000) {
    if (prec <= 1) {
      return Instance()->tr("%1 d").arg(qRound(ms / (24 * 60 * 60 * 1000.0)));
    }
    int primeValue = ms / (24 * 60 * 60 * 1000);
    int secondValue = (ms % (24 * 60 * 60 * 1000));
    if (prec <= 2) {
      int secondValueHour = qRound(secondValue / (60 * 60 * 1000.0));
      if (secondValueHour >= 24) {
        primeValue++;
        secondValueHour = 0;
      }
      return Instance()->tr("%1 d %2 h").arg(primeValue).arg(secondValueHour);
    } else {
      return Instance()->tr("%1 d %2").arg(primeValue).arg(FormatTimeTr(secondValue, prec - 1));
    }
  } else if (ms > 60 * 60 * 1000) {
    if (prec <= 1) {
      return Instance()->tr("%1 h").arg(qRound(ms / (60 * 60 * 1000.0)));
    }
    int primeValue = ms / (60 * 60 * 1000);
    int secondValue = (ms % (60 * 60 * 1000));
    if (prec <= 2) {
      int secondValueMin = qRound(secondValue / (60 * 1000.0));
      if (secondValueMin == 60) {
        primeValue++;
        secondValueMin = 0;
      }
      return Instance()->tr("%1:%2 h").arg(primeValue).arg(secondValueMin, 2, 10, QChar('0'));
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
        return Instance()->tr("1 h");
      }
      return Instance()->tr("%1 m").arg(primeValueMin);
    }
    int primeValue = ms / (60 * 1000);
    int secondValue = (ms % (60 * 1000));
    if (prec <= 2) {
      int secondValueSec = qRound(secondValue / 1000.0);
      if (secondValueSec == 60) {
        primeValue++;
        secondValueSec = 0;
        if (primeValue == 60) {
          return Instance()->tr("1:00 h");
        }
      }
      return Instance()->tr("%1:%2 m").arg(primeValue).arg(secondValueSec, 2, 10, QChar('0'));
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
            return Instance()->tr("1:00:00 h");
          }
        }
      }
      return QString("%1:%2.%3").arg(primeValue).arg(primeValue2, 2, 10, QChar('0')).arg(secondValueMs, 3, 10, QChar('0'));
    }
  } else if (ms > 1000) {
    if (prec <= 1) {
      int primeSec = qRound(ms / (1000.0));
      if (primeSec == 60) {
        return Instance()->tr("1:00 m");
      }
      return Instance()->tr("%1 s").arg(primeSec);
    }
    qreal value = (float)ms / (1.0f * 1000);
    if (value > 10) {
      return Instance()->tr("%1 s").arg(value, 4, 'f', 1);
    } else {
      return Instance()->tr("%1 s").arg(value, 4, 'f', 2);
    }
  } else {
    return Instance()->tr("%1 ms").arg(ms);
  }
}

FormatTr::FormatTr(QObject* parent)
  : QObject(parent)
{
  Q_INIT_RESOURCE(Common);

  if (!mSelf) {
    mSelf = this;

    QTranslator* translator = new QTranslator(this);
    translator->load(":/Tr/Common_ru.qm");
    QCoreApplication::instance()->installTranslator(translator);
  }
}
