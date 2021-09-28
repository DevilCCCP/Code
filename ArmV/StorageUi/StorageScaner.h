#pragma once

#include <QThread>
#include <QDateTime>

#include <Lib/Include/Common.h>

#include "Info.h"


DefineClassS(Container);

class StorageScaner: public QThread
{
  const ContInfo    mContInfo;

  volatile bool     mStop;
  ContainerS        mContainer;
  CellInfoList      mCellInfoList;

  Q_OBJECT

public:
  const CellInfoList& GetCellInfoList() const { return mCellInfoList; }

protected:
  virtual void run() override;

public:
  void Stop();

private:
  bool Scan();

signals:
  void PercentChanged(int percent);

public:
  StorageScaner(const ContInfo& _ContInfo, QObject* parent = 0);
};
