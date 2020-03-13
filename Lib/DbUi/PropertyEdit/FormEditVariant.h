#pragma once

#include <QWidget>


class FormEditVariant: public QWidget
{
  Q_OBJECT

public:
  explicit FormEditVariant(QWidget* parent = 0);
  ~FormEditVariant();

public:
  /*new */virtual void SetValues(const QString& _MinValue, const QString& _MaxValue);
  /*new */virtual void SetCurrent(const QVariant& data) = 0;
  /*new */virtual QVariant GetCurrent() = 0;

protected:
  void EditDone();

signals:
  void Done();
};
