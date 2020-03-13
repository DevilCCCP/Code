#pragma once

#include <QSpinBox>


class QSpinBoxZ: public QSpinBox
{
  int mZeroLength;

  Q_PROPERTY(int zeroLength READ zeroLength WRITE setZeroLength)

public:
  int zeroLength() const { return mZeroLength; }
  void setZeroLength(int zeroLength) { mZeroLength = zeroLength; }

protected:
  /*override */virtual QString textFromValue(int value) const override;

public:
  QSpinBoxZ(QWidget* parent = nullptr);
};
