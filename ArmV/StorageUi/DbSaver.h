#pragma once

#include <QThread>
#include <QDateTime>

#include <Lib/Include/Common.h>

#include "Info.h"


DefineClassS(Db);
DefineClassS(Container);
DefineClassS(DbIndex);

class DbSaver: public QThread
{
  const ContInfo          mContInfo;
  const QString           mDbConnection;

  volatile bool           mStop;
  ContainerS              mContainer;
  DbS                     mDb;
  DbIndexS                mDbIndex;

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
  bool CreateIndexes();

signals:
  void PercentChanged(int percent);

public:
  DbSaver(const ContInfo _ContInfo, const QString& _DbConnection, QObject* parent = 0);
};
