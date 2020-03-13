#include <QTimeZone>

#include <Lib/Db/Db.h>

#include "FormTableTime.h"
#include "ui_FormTableTime.h"


FormTableTime::FormTableTime(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormTableTime)
  , mFrom(QDateTime::currentDateTimeUtc()), mTo(QDateTime::currentDateTimeUtc()), mManual(false)
{
  ui->setupUi(this);

  QList<QByteArray> ids = QTimeZone::availableTimeZoneIds();
  foreach (QByteArray id, ids) {
    ui->comboBoxTimezone->addItem(id);
  }
  mTimezoneId = QTimeZone::systemTimeZoneId();
  ui->comboBoxTimezone->setCurrentText(mTimezoneId);

  UpdateTime();
}

FormTableTime::~FormTableTime()
{
  delete ui;
}


void FormTableTime::Clear()
{
  mFrom = QDateTime::currentDateTimeUtc();
  mTo = QDateTime::currentDateTimeUtc();
}

bool FormTableTime::GetWhere(const QString& column, QString& where)
{
  if (!mFrom.isValid() || !mTo.isValid()) {
    where.clear();
    return false;
  }

  where = QString("%0 >= %1 AND %0 <= %2").arg(column, ToSql(mFrom), ToSql(mTo));
  return true;
}

void FormTableTime::UpdateTime()
{
  QTimeZone timezone(mTimezoneId);
  mManual = true;
  ui->dateTimeEditFrom->setDateTime(mFrom.toOffsetFromUtc(timezone.offsetFromUtc(mFrom)));
  ui->dateTimeEditTo->setDateTime(mTo.toOffsetFromUtc(timezone.offsetFromUtc(mTo)));
  mManual = false;
}

void FormTableTime::on_comboBoxTimezone_currentIndexChanged(const QString& timezoneId)
{
  mTimezoneId = timezoneId.toUtf8();

  UpdateTime();
}

void FormTableTime::on_dateTimeEditFrom_dateTimeChanged(const QDateTime& dateTime)
{
  if (mManual) {
    return;
  }

  QDateTime utcDateTime = dateTime;
  utcDateTime.setOffsetFromUtc(0);
  mFrom = utcDateTime;
}

void FormTableTime::on_dateTimeEditTo_dateTimeChanged(const QDateTime& dateTime)
{
  if (mManual) {
    return;
  }

  QDateTime utcDateTime = dateTime;
  utcDateTime.setOffsetFromUtc(0);
  mTo = utcDateTime;
}
