#pragma once

#include <QWidget>
#include <QDateTime>


namespace Ui {
class FormTableTime;
}

class FormTableTime: public QWidget
{
  Ui::FormTableTime* ui;

  QDateTime           mFrom;
  QDateTime           mTo;
  QByteArray          mTimezoneId;
  bool                mManual;

  Q_OBJECT

public:
  explicit FormTableTime(QWidget* parent = 0);
  ~FormTableTime();

public:
  void Clear();
  bool GetWhere(const QString& column, QString& where);

private:
  void UpdateTime();

private slots:
  void on_comboBoxTimezone_currentIndexChanged(const QString& timezoneId);
  void on_dateTimeEditFrom_dateTimeChanged(const QDateTime& dateTime);
  void on_dateTimeEditTo_dateTimeChanged(const QDateTime& dateTime);
};
