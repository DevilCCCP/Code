#pragma once

#include <QTimeEdit>


class TimeEdit2: public QTimeEdit
{
protected:
  /*override */virtual QValidator::State validate(QString& input, int& pos) const override;

public:
  explicit TimeEdit2(QWidget* parent = 0);
};
