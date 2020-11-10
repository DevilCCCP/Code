#include "FormSchedule.h"
#include "ui_FormSchedule.h"


FormSchedule::FormSchedule(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormSchedule)
{
  Q_INIT_RESOURCE(Ui);

  ui->setupUi(this);

  ui->toolButtonUndo->setDefaultAction(ui->actionUndo);
  ui->toolButtonRedo->setDefaultAction(ui->actionRedo);
  ui->scheduleWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
  ui->scheduleWidget->addAction(ui->actionUndo);
  ui->scheduleWidget->addAction(ui->actionRedo);

  Clear();

  connect(ui->scheduleWidget, &ScheduleWidget::ScheduleChanged, this, &FormSchedule::OnScheduleChanged);
  connect(ui->scheduleWidget, &ScheduleWidget::ChangeCurrentTime, this, &FormSchedule::OnCurrentTimeChanged);
  connect(ui->scheduleWidget, &ScheduleWidget::ChangeCurrentObject, this, &FormSchedule::OnCurrentObjectChanged);
}

FormSchedule::~FormSchedule()
{
  delete ui;
}


void FormSchedule::SetScheduleInfo(const QStringList& _Info, int _TimePrecisionSec)
{
  mInfo = _Info;
  mTimePrecisionSec = _TimePrecisionSec;

  ui->timeEditPrecision->setMinimumTime(QTime::fromMSecsSinceStartOfDay(mTimePrecisionSec * 1000));
  ui->timeEditPrecision->setMaximumTime(QTime::fromMSecsSinceStartOfDay(2 * 60 * 60 * 1000));
  ui->timeEditPrecision->setTime(QTime::fromMSecsSinceStartOfDay(mTimePrecisionSec * 1000));
  mInfoColorList.clear();

  for (int i = 0; i < mInfo.size(); i++) {
    int hue = 0 + (2*i + 1) * 300 / (2 * mInfo.size());
    int saturation = 200;
    int value = 150;
    QColor infoColor;
    infoColor.setHsv(hue, saturation, value);
    mInfoColorList.append(infoColor);
  }

  ui->scheduleWidget->SetInfo(mInfo, mInfoColorList);
  ui->scheduleWidget->SetPrecision(mTimePrecisionSec);
}

void FormSchedule::SetSchedule(const Schedule& _Schedule)
{
  PushUndo();

  mSchedule = _Schedule;
  ui->scheduleWidget->SetSchedule(mSchedule);
}

const Schedule& FormSchedule::GetSchedule()
{
  return mSchedule;
}

void FormSchedule::Clear()
{
  mUndo.clear();
  mUndoIndex = -1;

  ui->actionUndo->setEnabled(false);
  ui->actionRedo->setEnabled(false);
}

void FormSchedule::PushUndo()
{
  if (mUndoIndex >= 0) {
    mUndo = mUndo.mid(0, mUndoIndex + 1);
  } else {
    mUndo.append(mSchedule);
  }
  mUndoIndex = -1;

  ui->actionUndo->setEnabled(true);
  ui->actionRedo->setEnabled(false);
}

void FormSchedule::DoUndo()
{
  if (mUndoIndex == 0 || mUndo.isEmpty()) {
    return;
  }

  if (mUndoIndex < 0) {
    mUndoIndex = mUndo.size() - 1;
    mUndo.append(mSchedule);
  } else {
    mUndoIndex--;
  }
  mSchedule = mUndo.at(mUndoIndex);
  ui->scheduleWidget->SetSchedule(mSchedule);

  ui->actionUndo->setEnabled(mUndoIndex > 0);
  ui->actionRedo->setEnabled(true);
}

void FormSchedule::DoRedo()
{
  if (mUndoIndex >= mUndo.size() - 1) {
    return;
  }

  mUndoIndex++;
  mSchedule = mUndo.at(mUndoIndex);
  ui->scheduleWidget->SetSchedule(mSchedule);

  ui->actionUndo->setEnabled(true);
  ui->actionRedo->setEnabled(mUndoIndex < mUndo.size() - 1);
}

void FormSchedule::OnScheduleChanged()
{
  PushUndo();

  mSchedule = ui->scheduleWidget->GetSchedule();
}

void FormSchedule::OnCurrentTimeChanged(int secs)
{
  if (secs < 0) {
    ui->labelCurrentTime->setText(QString());
    return;
  }

  int hours = secs / (60 * 60);
  int secsAdd = secs % (60 * 60);
  int minutes = secsAdd / 60;
  int seconds = secsAdd % 60;
  QString timeText = seconds
      ? QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'))
      : QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));
  ui->labelCurrentTime->setText(timeText);
}

void FormSchedule::OnCurrentObjectChanged(int object)
{
  if (object < 0) {
    ui->labelCurrentObject->setText(QString());
    return;
  }

  QString objectText = QString("<p><span style=\"color:%2;\">%1</span></p>").arg(mInfo.value(object), mInfoColorList.value(object).name());
  ui->labelCurrentObject->setText(objectText);
}

void FormSchedule::on_timeEditPrecision_timeChanged(const QTime& time)
{
  ui->scheduleWidget->SetPrecision(time.msecsSinceStartOfDay() / 1000);
}

void FormSchedule::on_actionUndo_triggered()
{
  DoUndo();
}

void FormSchedule::on_actionRedo_triggered()
{
  DoRedo();
}
