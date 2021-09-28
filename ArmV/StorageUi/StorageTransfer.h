#pragma once

#include <QThread>
#include <QDateTime>

#include <Lib/Include/Common.h>

#include "Info.h"


DefineClassS(Container);

class StorageTransfer: public QThread
{
  const QVector<ContInfo> mContInfoSources;
  const ContInfo          mContInfoDest;
  const CellsVector       mCellsVector;

  volatile bool           mStop;
  QList<ContainerS>       mContainerSources;
  ContainerS              mContainerDest;

  bool                    mSuccess;
  QString                 mErrorString;

  Q_OBJECT

public:
  bool Success() const { return mSuccess; }
  const QString& ErrorString() const { return mErrorString; }

protected:
  virtual void run() override;

public:
  void Stop();

private:
  bool Transfer();

signals:
  void PercentChanged(int percent);

public:
  StorageTransfer(const QVector<ContInfo>& _ContInfoSources, const ContInfo& _ContInfoDest, const CellsVector& _CellsVector, QObject* parent = 0);
};
