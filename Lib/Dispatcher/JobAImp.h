#pragma once

#include <QElapsedTimer>

#include <Lib/Db/Db.h>
#include <Lib/Db/Job.h>

#include "ImpD.h"


DefineClassS(JobAImp);

class JobAImp: public ImpD
{
  PROPERTY_GET_SET(bool, Debug)

  bool                   mInit;
  PROPERTY_GET(DbItemBS, Job)
  PROPERTY_GET(qint64,   ActionId)
  PROPERTY_GET(qint64,   JobIterFrom)
  PROPERTY_GET(qint64,   JobIterTo)
  PROPERTY_GET(int,      JobTry)
  QElapsedTimer          mUpdateTimer;
  qint64                 mNextUpdate;

public:
  /*override */virtual const char* Name() override { return "JobAImp"; }
  /*override */virtual const char* ShortName() override { return "J"; }
protected:
//  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
//  /*override */virtual void DoRelease() override;

protected:
  /*override */virtual bool LoadSettings(SettingsA* settings) override;

protected:
  /*new */virtual QString JobName() = 0;
  /*new */virtual DbTableBS JobTable() = 0;

  /*new */virtual qint64 IterationsPerAction() = 0;
  /*new */virtual qint64 LastIteration();
  /*new */virtual qint64 JobMaximumSecs();
  /*new */virtual int JobMaxTries();
  /*new */virtual qint64 JobUpdateMs();

  /*new */virtual bool DoJob(bool& result) = 0;
  /*new */virtual bool UpdateJob() = 0;

private:
  bool DoUpdate();
  bool DoAction();
  bool TakeAction();
  bool ApplyAction(bool result);
  bool CancelAction();

  bool JobTake(const QString& function, const QStringList& args);

public:
  JobAImp(const Db& _Db, int _WorkPeriodMs = -1);
};
