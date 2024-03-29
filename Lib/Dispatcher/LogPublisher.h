#pragma once

#include <QMutex>
#include <QElapsedTimer>

#include <Lib/Dispatcher/ImpD.h>


DefineClassS(LogPublisher);

class LogPublisher: public ImpD
{
  DbS                mLogDb;
  int                mLogTruncHours;

  struct WorkerLog {
    QDateTime          StartTime;
    QDateTime          EndTime;
    QList<WorkerStatS> StatList;
  };
  QList<WorkerLog>   mLogList;
  QMutex             mLogMutex;

  QElapsedTimer      mTruncTimer;
  qint64             mNextTrunc;

public:
  /*override */virtual const char* Name() override { return "Logger"; }
  /*override */virtual const char* ShortName() override { return "Lg"; }
protected:
  /*override */virtual bool LoadSettings(SettingsA* settings) override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;

public:
  void PushLog(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList);

private:
  void ProcessLogs();
  bool PublishLog(const QDateTime& startTime, const QDateTime& endTime, const QList<WorkerStatS>& statList);
  bool GetFirstLog(QDateTime& startTime, QDateTime& endTime, QList<WorkerStatS>& statList);
  void PopLog();

public:
  LogPublisher(const DbS& _LogDb);
};
