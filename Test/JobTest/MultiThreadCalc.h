#ifndef MULTITHREADCALC_H
#define MULTITHREADCALC_H

#include <QObject>


class MultiThreadCalc : public QObject
{
  Q_OBJECT
public:
  explicit MultiThreadCalc(QObject *parent = 0);

signals:

public slots:
  void Calc(qint64 circles, int from, int to);
};

#endif // MULTITHREADCALC_H
