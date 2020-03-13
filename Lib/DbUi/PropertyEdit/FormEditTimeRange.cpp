#include "FormEditTimeRange.h"
#include "DialogText.h"
#include "TimeEdit2.h"
#include "ui_FormEditTimeRange.h"


void FormEditTimeRange::SetCurrent(const QVariant& data)
{
  QString text = data.toString();
  QStringList rangePair = text.split('-');
  if (rangePair.size() == 2) {
    ApplyRange(rangePair.at(0), ui->timeEditFrom);
    ApplyRange(rangePair.at(1), ui->timeEditTo);
  }
}

QVariant FormEditTimeRange::GetCurrent()
{
  QString from = ui->timeEditFrom->text();
  QString to = ui->timeEditTo->text();
  int secs = ui->timeEditFrom->time().secsTo(ui->timeEditTo->time());
  if (secs <= 0) {
    int mto = (24*60*60 + QTime(0, 0).secsTo(ui->timeEditTo->time())) / 60;
    int h = mto / 60;
    int m = mto % 60;
    to = QString("%1:%2").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));
  }
  return QVariant(QString("%1-%2").arg(from, to));
}


FormEditTimeRange::FormEditTimeRange(QWidget* parent)
  : FormEditVariant(parent)
  , ui(new Ui::FormEditTimeRange)
{
  ui->setupUi(this);

#ifdef LANG_EN
  ui->labelFrom->setText("From");
  ui->labelTo->setText("to");
#else
  ui->labelFrom->setText("С");
  ui->labelTo->setText("по");
#endif
  setFocusProxy(ui->timeEditFrom);
}

FormEditTimeRange::~FormEditTimeRange()
{
  delete ui;
}


void FormEditTimeRange::ApplyRange(const QString& text, QTimeEdit* timeEdit)
{
  QStringList hm = text.split(':');
  if (hm.size() == 2) {
    int h = (hm.at(0).toInt() % 24);
    int m = hm.at(1).toInt();
    timeEdit->setTime(QTime(h, m));
  }
}

