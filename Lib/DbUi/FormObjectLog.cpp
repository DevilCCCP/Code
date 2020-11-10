#include <QDateTime>
#include <QLineEdit>
#include <QRegExp>

#include <Lib/Common/FormatTr.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ObjectLog.h>

#include "FormObjectLog.h"
#include "ui_FormObjectLog.h"


FormObjectLog::FormObjectLog(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormObjectLog)
  , mDb(nullptr)
{
  Q_INIT_RESOURCE(DbUi);
  Q_INIT_RESOURCE(Ui);

  QTranslator* translator = new QTranslator(this);
  translator->load(":/Tr/DbUi_ru.qm");
  QApplication::instance()->installTranslator(translator);

  new FormatTr(this);

  ui->setupUi(this);
  setMouseTracking(true);

  ui->toolButtonMoveLeft->setDefaultAction(ui->actionBackward);
  ui->toolButtonMoveRight->setDefaultAction(ui->actionForward);
  ui->toolButtonMoveNow->setDefaultAction(ui->actionCurrent);

  this->addAction(ui->actionBackward);
  this->addAction(ui->actionForward);
  this->addAction(ui->actionCurrent);
  this->setContextMenuPolicy(Qt::ActionsContextMenu);

  ui->comboBoxTimePeriod->addItem(tr("12 h"));
  ui->comboBoxTimePeriod->addItem(tr("24 h"));
  ui->comboBoxTimePeriod->addItem(tr("2 d"));
  ui->comboBoxTimePeriod->addItem(tr("7 d"));
  ui->comboBoxTimePeriod->setCurrentIndex(1);

  MoveTime(QDateTime::currentDateTime());
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

void FormObjectLog::MoveTime(const QDateTime& toTime)
{
  mToTime = toTime;
  mFromTime = mToTime.addSecs(-mPeriodSecs);
  QSignalBlocker b(ui->dateTimeEditFrom);
  ui->dateTimeEditFrom->setDateTime(mFromTime);
  QSignalBlocker b2(ui->dateTimeEditTo);
  ui->dateTimeEditTo->setDateTime(mToTime);

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
  QString periodText = value.toLower();
  if (valueRegExp.exactMatch(periodText)) {
    bool ok = false;
    int newPeriodSecs = valueRegExp.cap(1).toInt(&ok);
    if (ok) {
      QString suffix = valueRegExp.cap(2);
      if (suffix == "m" || suffix == "м") {
        newPeriodSecs *= 60;
      } else if (suffix == "h" || suffix == "ч") {
        newPeriodSecs *= 60 * 60;
      } else if (suffix == "d" || suffix == "д") {
        newPeriodSecs *= 60 * 60 * 24;
      }

      if (newPeriodSecs >= 1 * 60 * 60) {
        mPeriodSecs = newPeriodSecs;
        mFromTime = mToTime.addSecs(-mPeriodSecs);
        QSignalBlocker b(ui->dateTimeEditFrom);
        ui->dateTimeEditFrom->setDateTime(mFromTime);

        Update();

        ui->comboBoxTimePeriod->lineEdit()->setStyleSheet("color: rgb(0, 85, 0);");
        return;
      }
    }
  }
  ui->comboBoxTimePeriod->lineEdit()->setStyleSheet("color: rgb(170, 0, 0);");
}

void FormObjectLog::on_actionBackward_triggered()
{
  MoveTime(mToTime.addSecs(-mPeriodSecs));
}

void FormObjectLog::on_actionForward_triggered()
{
  MoveTime(mToTime.addSecs(mPeriodSecs));
}

void FormObjectLog::on_actionCurrent_triggered()
{
  MoveTime(QDateTime::currentDateTime());
}
