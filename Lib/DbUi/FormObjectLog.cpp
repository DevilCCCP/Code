#include <QDateTime>
#include <QRegExp>

#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectLog.h>

#include "FormObjectLog.h"
#include "ui_FormObjectLog.h"


const int kDefaultTimePeriodSecs = 24 * 60 * 60;

FormObjectLog::FormObjectLog(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormObjectLog)
  , mDb(nullptr)
{
  ui->setupUi(this);
  setMouseTracking(true);

  mPeriodSecs = kDefaultTimePeriodSecs;
  mFromTime   = QDateTime::currentDateTime().addSecs(-mPeriodSecs);
  mToTime     = mFromTime.addSecs(mPeriodSecs);

  ui->dateTimeEditFrom->setDateTime(mFromTime);
  ui->dateTimeEditTo->setDateTime(mToTime);
}

FormObjectLog::~FormObjectLog()
{
  delete ui;
}


void FormObjectLog::Init(Db* _Db, const LogSchema& _LogSchema)
{
  mDb = _Db;

  ui->objectLogWidget->SetLogSchema(_LogSchema);
}

void FormObjectLog::SetTimePeriod(const QDateTime& startTime, int periodSecs)
{
  mPeriodSecs = periodSecs;
  mFromTime   = startTime;
  mToTime     = mFromTime.addSecs(mPeriodSecs);

  ui->dateTimeEditFrom->setDateTime(mFromTime);
  ui->dateTimeEditTo->setDateTime(mToTime);

  Update();
}

void FormObjectLog::SetObjectsList(const QList<ObjectItemS>& objectList)
{
  mObjectList = objectList;

  Update();
}

void FormObjectLog::Update()
{
  ui->objectLogWidget->SetTimePeriod(mFromTime, mToTime);
  ui->labelError->setVisible(false);
  QVector<ObjectLogS> logList;
  foreach (const ObjectItemS& item, mObjectList) {
    if (!mDb->getObjectLogTable()->Select(QString("WHERE _object = %1 AND period_start > %2 AND period_start < %3 ORDER BY period_start, _id")
                                          .arg(item->Id).arg(ToSql(mFromTime), ToSql(mToTime)), logList)) {
      logList.clear();
      ui->labelError->setVisible(true);
      break;
    }
  }

  ui->objectLogWidget->SetObjectLog(mObjectList, logList);
  ui->objectLogWidget->update();
}

void FormObjectLog::on_dateTimeEditFrom_dateTimeChanged(const QDateTime& dateTime)
{
  mFromTime = dateTime;

  Update();
}

void FormObjectLog::on_dateTimeEditTo_dateTimeChanged(const QDateTime& dateTime)
{
  mToTime = dateTime;

  Update();
}

void FormObjectLog::on_comboBoxTimePeriod_editTextChanged(const QString& value)
{
  QRegExp valueRegExp("^\\s*(\\d*)\\s*(\\w*)\\s*$");
  if (valueRegExp.exactMatch(value)) {
    bool ok = false;
    mPeriodSecs = valueRegExp.cap(1).toInt(&ok);
    if (ok) {
      QString suffix = valueRegExp.cap(2);
      if (suffix == "m") {
        mPeriodSecs *= 60;
      } else if (suffix == "h") {
        mPeriodSecs *= 60 * 60;
      } else if (suffix == "d") {
        mPeriodSecs *= 60 * 60 * 24;
      }

      mFromTime = mToTime.addSecs(-mPeriodSecs);

      ui->dateTimeEditFrom->setDateTime(mFromTime);
    }
  }
}
