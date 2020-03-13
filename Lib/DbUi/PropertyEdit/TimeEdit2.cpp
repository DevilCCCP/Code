#include "TimeEdit2.h"


QValidator::State TimeEdit2::validate(QString& input, int& pos) const
{
  if (input == "24:00") {
    return QValidator::Acceptable;
  }
  return QTimeEdit::validate(input, pos);
}


TimeEdit2::TimeEdit2(QWidget* parent)
  : QTimeEdit(parent)
{
  //setMaximumTime(QTime(24*60*60*1000));
}
