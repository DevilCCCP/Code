#pragma once

#include <QDateTime>

#include <Lib/Dispatcher/ImpD.h>
#include <Lib/Settings/Schedule.h>


DefineClassS(ReporterA);
DefineClassS(Db);
DefineClassS(ObjectTypeTable);
DefineClassS(Report);
DefineClassS(ReportTable);
DefineClassS(ReportSend);
DefineClassS(ReportSendTable);
DefineClassS(Smtp);

struct Email {
  int           Id;
  QString       Username;
  QString       Addr;
  int           DelayMs;
  ReportSendS   LastReportSend;
};

class ReporterA: public ImpD
{
  const Db&        mDb;
  ReportTableS     mReportTable;
  ReportSendTableS mReportSendTable;
  FilesTableS      mFilesTable;
  ReportFilesMapS  mReportFilesMap;

protected:
  enum EPeriodic {
    eDayly  = 0,
    eWeekly = 1
  };
  struct Range {
    QDateTime From;
    QDateTime To;

    Range(const QDateTime& _From, const QDateTime& _To): From(_From), To(_To) { }
  };
  typedef QList<Range> Ranges;

private:
  QMap<int, int>    mAccountMap;
  QMap<int, SmtpS>  mAccounts;
  QMap<int, Email>  mEmails;

  TimePeriod        mPeriodDay;
  DayWPeriod        mPeriodWeek;
  QString           mReporterName;
  bool              mIsDayly;
  bool              mIsWeekly;
  QDate             mStartDay;
  int               mCreateDaylyMs;
  int               mCreateWeeklyMs;

  QDateTime         mStartTime;
  QDateTime         mNextDaylyBegin;
  QDateTime         mNextDaylyEnd;
  QDateTime         mNextDaylyCreate;
  QDateTime         mNextWeeklyBegin;
  QDateTime         mNextWeeklyEnd;
  QDateTime         mNextWeeklyCreate;
  bool              mInit;

  qint64            mLatestReportId;
  QByteArray        mReportData;
  QList<FilesS>     mReportFiles;
  ReportS           mCurrentReport;
  QList<FilesS>     mCurrentReportFiles;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Reporter"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "R"; }
protected:
  /*override */virtual bool LoadSettings(SettingsA* settings) Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;

protected:
  /*new */virtual bool InitReport(QDateTime& startTime);
  /*new */virtual bool CreateReport(EPeriodic periodic, const Ranges& ranges, QByteArray& reportData, QList<FilesS>& reportFiles) = 0;
  /*new */virtual QString MakeReportSubject(const ReportS& report);

private:
  bool LoadRecepients();
  bool LoadAccount(int id);
  bool LoadEmail(int id);
  bool LoadLatestReport();
  bool StoreReport(EPeriodic periodic, const QDateTime& periodBegin, const QDateTime& periodEnd);

  void DoDaylyReport();
  void DoWeeklyReport();
  void DoSendReports();
  void DoSendOne(const SmtpS& smtp, Email* email);
  bool DoSendOneReport(const SmtpS& smtp, Email* email);
  bool LoadNextReport(const qint64& lastId = 0);

  bool Prepare();
  bool Prepare(EPeriodic periodic, QDateTime& nextPeriod);
  void CorrectBeginDay(QDateTime& time);
  void CorrectEndDay(QDateTime& time);
  void CorrectBeginWeek(QDateTime& time);
  void CorrectEndWeek(QDateTime& time);

public:
  ReporterA(const Db& _Db);
  /*override */virtual ~ReporterA();
};

