#include <QLocale>
#include <QRegExp>

#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/Report.h>
#include <Lib/Db/ReportSend.h>
#include <Lib/Db/Files.h>
#include <Lib/Db/ReportFilesMap.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Settings/SettingsA.h>
#include <Lib/Smtp/Smtp.h>
#include <Lib/Log/Log.h>

#include "ReporterA.h"


const int kWorkPeriodMs = 500;
const int kCreateDelayDefaultMs = 5 * 60 * 1000;

bool ReporterA::LoadSettings(SettingsA* settings)
{
  QString periodD = settings->GetValue("PeriodD", "0:00-24:00").toString();
  Schedule::ParseTimePeriod(periodD, mPeriodDay);
  QString periodW = settings->GetValue("PeriodW", "1-7").toString();
  Schedule::ParseDayWPeriod(periodW, mPeriodWeek);
  mReporterName = settings->GetValue("Reporter", "Report service").toString();
  mIsDayly = settings->GetValue("Dayly", true).toBool();
  mIsWeekly = settings->GetValue("Weekly", true).toBool();
  if (mIsDayly) {
    mCreateDaylyMs = settings->GetValue("DaylyAfter", kCreateDelayDefaultMs).toInt();
  } else {
    mCreateDaylyMs = kCreateDelayDefaultMs;
  }
  if (mIsWeekly) {
    mCreateWeeklyMs = settings->GetValue("WeeklyAfter", kCreateDelayDefaultMs).toInt();
  } else {
    mCreateWeeklyMs = kCreateDelayDefaultMs;
  }
  QString startDay = settings->GetValue("Start", "01-01-2000").toString();
  mStartDay = QDate::fromString(startDay, "dd-MM-yyyy");
  if (!mStartDay.isValid()) {
    mStartDay = QDate::fromString(startDay, "d-M-yyyy");
    if (!mStartDay.isValid()) {
      Log.Warning(QString("Start time is invalid, using 'too old' time"));
      mStartDay.setDate(2000, 1, 1);
    }
  }

  return LoadRecepients();
}

bool ReporterA::DoCircle()
{
  if (!mInit) {
    if (!Prepare()) {
      return true;
    }
    mInit = true;
  }

  if (mIsDayly) {
    DoDaylyReport();
  }
  if (mIsWeekly) {
    DoWeeklyReport();
  }

  DoSendReports();

  return true;
}

void ReporterA::DoRelease()
{
  mAccountMap.clear();
  mAccounts.clear();
  mEmails.clear();
}

bool ReporterA::InitReport(QDateTime& startTime)
{
  Q_UNUSED(startTime);

  return true;
}

QString ReporterA::MakeReportSubject(const ReportS& report)
{
  QString subjPrefix;
  QString timeFormat1;
  QString timeFormat2;
  switch ((EPeriodic)report->Type) {
  case eDayly:
    subjPrefix = "Ежедневный отчёт";
    timeFormat1 = "dd MMM hh:mm";
    timeFormat2 = report->PeriodEnd.date() > report->PeriodBegin.date()? "dd MMM hh:mm": "hh:mm";
    break;
  case eWeekly:
    subjPrefix = "Еженедельный отчёт";
    timeFormat1 = "dd MMM(ddd)";
    timeFormat2 = "dd MMM(ddd)";
    break;
  default:
    subjPrefix = "Отчёт";
    timeFormat1 = "dd MMM hh:mm";
    timeFormat2 = "dd MMM hh:mm";
    break;
  }
  QLocale locale = QLocale(QLocale::Russian, QLocale::RussianFederation);
  QString startTime = locale.toString(report->PeriodBegin, timeFormat1);
  QString endTime = locale.toString(report->PeriodEnd, timeFormat2);
  QString fullSubject = subjPrefix + QString(" за период [%1 - %2]").arg(startTime).arg(endTime);
  return fullSubject;
}

bool ReporterA::LoadRecepients()
{
  ObjectTypeS objectTypeTable(new ObjectTypeTable(mDb));
  int  smtpType = 0;
  int emailType = 0;

  if (const NamedItem* item = objectTypeTable->GetItemByName("smp")) {
    smtpType = item->Id;
  }
  if (const NamedItem* item = objectTypeTable->GetItemByName("eml")) {
    emailType = item->Id;
  }
  if (!smtpType || !emailType) {
    Log.Error(QString("Smtp and e-mail types not loaded from Db"));
    return false;
  }

  QSet<int> accounts;
  QSet<int> emails;
  mAccountMap.clear();
  auto q = mDb.MakeQuery();
  q->prepare("SELECT cs._omaster, cs._oslave FROM object_connection cm"
             " INNER JOIN object_connection cs ON cs._omaster = cm._oslave"
             " INNER JOIN object om ON om._id = cs._omaster"
             " INNER JOIN object os ON os._id = cs._oslave"
             " WHERE cm._omaster = ? AND om._otype = ? AND os._otype = ? AND os.status = 0;");
  q->addBindValue(GetOverseer()->Id());
  q->addBindValue(smtpType);
  q->addBindValue(emailType);
  if (!mDb.ExecuteQuery(q)) {
    return false;
  }
  while (q->next()) {
    int idMaster = q->value(0).toInt();
    int idSlave = q->value(1).toInt();
    mAccountMap.insertMulti(idMaster, idSlave);
    accounts.insert(idMaster);
    emails.insert(idSlave);
  }

  mAccounts.clear();
  for (auto itr = accounts.begin(); itr != accounts.end(); itr++) {
    if (!LoadAccount(*itr)) {
      return false;
    }
  }

  mEmails.clear();
  for (auto itr = emails.begin(); itr != emails.end(); itr++) {
    if (!LoadEmail(*itr)) {
      return false;
    }
  }

  if (!LoadLatestReport()) {
    return false;
  }
  return true;
}

bool ReporterA::LoadAccount(int id)
{
  DbSettings settings(mDb);
  if (!settings.Open(QString::number(id))) {
    Log.Error(QString("Can't get account settings (id: %1)").arg(id));
    return false;
  }

  SmtpS smtp(new Smtp());
  smtp->setSmtpServer(settings.GetMandatoryValue("Uri").toString());
  smtp->setSmtpPort(settings.GetMandatoryValue("Port").toInt());
  smtp->setSmtpType(settings.GetMandatoryValue("Type").toBool()? Smtp::ePlane: Smtp::eSsl);
  smtp->setUserName(settings.GetMandatoryValue("Username").toString());
  smtp->setUserPass(settings.GetMandatoryValue("Userpass").toString());
  smtp->setUserDomain(settings.GetMandatoryValue("Domain").toString());
  mAccounts[id] = smtp;
  return true;
}

bool ReporterA::LoadEmail(int id)
{
  DbSettings settings(mDb);
  if (!settings.Open(QString::number(id))) {
    Log.Error(QString("Can't get e-mail settings (id: %1)").arg(id));
    return false;
  }

  Email eml;
  eml.Id       = id;
  eml.Username = settings.GetMandatoryValue("Username").toString();
  eml.Addr     = settings.GetMandatoryValue("Usermail").toString();
  eml.DelayMs  = settings.GetValue("Delay", 0).toInt();
  mEmails[id] = eml;
  return true;
}

bool ReporterA::LoadLatestReport()
{
  QList<ReportS> reports;
  if (!mReportTable->Select(QString("WHERE _object = %2 ORDER BY _id DESC LIMIT 1").arg(GetOverseer()->Id()), reports)) {
    return false;
  }
  if (!reports.isEmpty()) {
    mLatestReportId = reports.first()->Id;
  } else {
    mLatestReportId = 0;
  }
  Log.Info(QString("Latest report is %1").arg(mLatestReportId));
  return true;
}

bool ReporterA::StoreReport(ReporterA::EPeriodic periodic, const QDateTime& periodBegin, const QDateTime& periodEnd)
{
  ReportS report(new Report());
  report->ObjectId = GetOverseer()->Id();
  report->Type = (int)periodic;
  report->PeriodBegin = periodBegin;
  report->PeriodEnd = periodEnd;
  report->Data = mReportData;
  DbTransactionS transaction = mDb.BeginTransaction();
  if (!transaction) {
    return false;
  }
  if (!mReportTable->Insert(report)) {
    return false;
  }
  foreach (const FilesS& file, mReportFiles) {
    file->ObjectId = GetOverseer()->Id();
    if (!mFilesTable->Insert(file)) {
      return false;
    }
    if (!mReportFilesMap->InsertItem(report->Id, file->Id)) {
      return false;
    }
  }
  if (!transaction->Commit()) {
    return false;
  }
  mLatestReportId = report->Id;
  mCurrentReport = report;
  mCurrentReportFiles.swap(mReportFiles);
  return true;
}

void ReporterA::DoDaylyReport()
{
  while (mNextDaylyCreate < QDateTime::currentDateTime()) {
    Ranges ranges;
    ranges.append(Range(mNextDaylyBegin, mNextDaylyEnd));
    mReportData.clear();
    mReportFiles.clear();
    if (!CreateReport(eDayly, ranges, mReportData, mReportFiles) || !StoreReport(eDayly, mNextDaylyBegin, mNextDaylyEnd)) {
      break;
    }
    Log.Info(QString("Created dayly report ('%1', '%2')").arg(mNextDaylyBegin.toString()).arg(mNextDaylyEnd.toString()));
    mNextDaylyBegin  = mNextDaylyBegin.addDays(1);
    mNextDaylyEnd    = mNextDaylyEnd.addDays(1);
    mNextDaylyCreate = mNextDaylyCreate.addDays(1);
  }
}

void ReporterA::DoWeeklyReport()
{
  while (mNextWeeklyCreate < QDateTime::currentDateTime()) {
    Ranges ranges;
    int fromDay = mNextWeeklyBegin.date().dayOfWeek();
    int toDay   = mNextWeeklyEnd.date().dayOfWeek();
    for (int i = 0; i < 7; i++) {
      if (mPeriodWeek.Days[i]) {
        int nowDay = i + 1;
        ranges.append(Range(mNextWeeklyBegin.addDays(nowDay - fromDay), mNextWeeklyEnd.addDays(nowDay - toDay)));
      }
    }
    ranges.append(Range(mNextWeeklyBegin, mNextWeeklyEnd));
    if (!CreateReport(eWeekly, ranges, mReportData, mReportFiles) || !StoreReport(eWeekly, mNextWeeklyBegin, mNextWeeklyEnd)) {
      break;
    }
    Log.Info(QString("Created weekly report ('%1', '%2')").arg(mNextWeeklyBegin.toString()).arg(mNextWeeklyEnd.toString()));
    mNextWeeklyBegin  = mNextWeeklyBegin.addDays(7);
    mNextWeeklyEnd    = mNextWeeklyEnd.addDays(7);
    mNextWeeklyCreate = mNextWeeklyCreate.addDays(7);
  }
}

void ReporterA::DoSendReports()
{
  if (!mLatestReportId) {
    return;
  }

  for (auto itr = mAccountMap.begin(); itr != mAccountMap.end(); itr++) {
    const SmtpS& smtp  = mAccounts[itr.key()];
    Email* email = &mEmails[itr.value()];
    DoSendOne(smtp, email);
  }
}

void ReporterA::DoSendOne(const SmtpS& smtp, Email* email)
{
  if (!email->LastReportSend) {
    QList<ReportSendS> reportSends;
    if (!mReportSendTable->Select(QString("WHERE _oto = %1").arg(email->Id), reportSends)) {
      return;
    }

    if (!reportSends.isEmpty()) {
      email->LastReportSend = reportSends.first();
    } else {
      email->LastReportSend.reset(new ReportSend());
      email->LastReportSend->OtoId = email->Id;
      email->LastReportSend->LastReportId = 0;
    }
  }

  while (email->LastReportSend->LastReportId < mLatestReportId) {
    if (email->LastReportSend->SendTime.toMSecsSinceEpoch() + email->DelayMs > QDateTime::currentMSecsSinceEpoch()) {
      return;
    }

    if (!mCurrentReport || mCurrentReport->Id != email->LastReportSend->Id + 1) {
      LoadNextReport(email->LastReportSend->LastReportId);
    }
    if (!mCurrentReport) {
      return;
    }

    Log.Info(QString("Send report (from: '%1', to: '%2', id: %3, latest: %4").arg(smtp->getUserName()).arg(email->Username).arg(mCurrentReport->Id).arg(mLatestReportId));
    if (!DoSendOneReport(smtp, email)) {
      return;
    }
    email->LastReportSend->LastReportId = mCurrentReport->Id;
    email->LastReportSend->SendTime = QDateTime::currentDateTime();
    mReportSendTable->Update(email->LastReportSend);
  }
}

bool ReporterA::DoSendOneReport(const SmtpS& smtp, Email* email)
{
  SmtpMail mail;
  QString fromEmail = (smtp->getUserName().contains("@"))? smtp->getUserName(): smtp->getUserName() + "@" + smtp->getUserDomain();

  if (!mReporterName.isEmpty()) {
    mail.setFrom(QString("%1 <%2>").arg(mReporterName).arg(fromEmail));
  } else {
    mail.setFrom(fromEmail);
  }
  mail.setTo(QStringList() << QString("%1 <%2>").arg(email->Username).arg(email->Addr));
  QString subj = MakeReportSubject(mCurrentReport);
  mail.setSubject(subj);
  mail.setBody(QString::fromUtf8(mCurrentReport->Data));
  mail.setBodyHtml(true);
  foreach (const FilesS& file, mCurrentReportFiles) {
    SmtpMail::Attach attach;
    attach.setName(file->Name);
    attach.setMimeType(file->MimeType);
    attach.setData(file->Data);
    mail.AddAttach(attach);
  }

  return smtp->SendMail(mail);
}

bool ReporterA::LoadNextReport(const qint64& lastId)
{
  mCurrentReport.clear();
  mCurrentReportFiles.clear();

  if (lastId >= mLatestReportId) {
    return false;
  }
  QString query = QString("WHERE _object = %2 AND _id > %1 ORDER BY _id LIMIT 1").arg(lastId).arg(GetOverseer()->Id());
  QList<ReportS> reports;
  if (!mReportTable->Select(query, reports) || reports.isEmpty()) {
    return false;
  }
  ReportS newReport = reports.first();
  QList<qint64> filesIds;
  if (!mReportFilesMap->GetChildIds(newReport->Id, filesIds)) {
    return false;
  }
  QList<FilesS> newReportFiles;
  foreach (const qint64& filesId, filesIds) {
    FilesS file;
    if (!mFilesTable->SelectById(filesId, file)) {
      return false;
    }
    if (file) {
      newReportFiles.append(file);
    }
  }

  mCurrentReport = newReport;
  mCurrentReportFiles.swap(newReportFiles);
  return true;
}

bool ReporterA::Prepare()
{
  if (!mNextDaylyBegin.isValid() && !Prepare(eDayly, mNextDaylyBegin)) {
    return false;
  }
  if (!mNextWeeklyBegin.isValid() && !Prepare(eWeekly, mNextWeeklyBegin)) {
    return false;
  }

  CorrectBeginDay(mNextDaylyBegin);
  CorrectBeginWeek(mNextWeeklyBegin);
  CorrectBeginDay(mNextWeeklyBegin);
  mNextDaylyEnd = mNextDaylyBegin;
  CorrectEndDay(mNextDaylyEnd);
  mNextDaylyCreate = mNextDaylyEnd.addMSecs(mCreateDaylyMs);
  mNextWeeklyEnd = mNextWeeklyBegin;
  CorrectEndWeek(mNextWeeklyEnd);
  CorrectEndDay(mNextWeeklyEnd);
  mNextWeeklyCreate = mNextWeeklyEnd.addMSecs(mCreateWeeklyMs);
  if (mIsDayly) {
    Log.Info(QString("Next dayly report will be [%1-%2] at %3")
             .arg(mNextDaylyBegin.toString()).arg(mNextDaylyEnd.toString()).arg(mNextDaylyCreate.toString()));
  }
  if (mIsWeekly) {
    Log.Info(QString("Next weekly report will be [%1-%2] at %3")
             .arg(mNextWeeklyBegin.toString()).arg(mNextWeeklyEnd.toString()).arg(mNextWeeklyCreate.toString()));
  }
  return true;
}

bool ReporterA::Prepare(ReporterA::EPeriodic periodic, QDateTime& nextPeriod)
{
  QString where = QString("WHERE _object=%1 AND type=%2"
                          " ORDER BY period_begin DESC LIMIT 1")
      .arg(GetOverseer()->Id()).arg((int)periodic);

  QList<ReportS> reports;
  if (!mReportTable->Select(where, reports)) {
    return false;
  }
  if (reports.isEmpty()) {
    if (!mStartTime.isValid()) {
      if (!InitReport(mStartTime)) {
        return false;
      }
      if (!mStartTime.isValid()) {
        mStartTime = QDateTime::currentDateTime();
      } else if (mStartDay > mStartTime.date()) {
        mStartTime = QDateTime(mStartDay);
      }
    }
    nextPeriod = mStartTime;
  } else {
    nextPeriod = reports.first()->PeriodBegin.addDays(periodic == eDayly? 1: 7);
  }
  return true;
}

void ReporterA::CorrectBeginDay(QDateTime& time)
{
  time.setTime(QTime(0, 0));
  time = time.addMSecs(mPeriodDay.FromMs);
}

void ReporterA::CorrectEndDay(QDateTime& time)
{
  time.setTime(QTime(0, 0));
  time = time.addMSecs(mPeriodDay.ToMs);
}

void ReporterA::CorrectBeginWeek(QDateTime& time)
{
  int nowDay = time.date().dayOfWeek();
  int setDay = 1;
  while (!mPeriodWeek.Days[setDay - 1] && setDay < 7) {
    setDay++;
  }
  int addDay = setDay - nowDay;
  time = time.addDays(addDay);
}

void ReporterA::CorrectEndWeek(QDateTime& time)
{
  int nowDay = time.date().dayOfWeek();
  int setDay = 7;
  while (!mPeriodWeek.Days[setDay - 1] && setDay > 1) {
    setDay--;
  }
  int addDay = setDay - nowDay;
  time = time.addDays(addDay);
}


ReporterA::ReporterA(const Db& _Db)
  : ImpD(_Db, kWorkPeriodMs)
  , mDb(_Db), mReportTable(new ReportTable(mDb)), mReportSendTable(new ReportSendTable(mDb))
  , mFilesTable(new FilesTable(mDb)), mReportFilesMap(new ReportFilesMap(mDb))
  , mInit(false)
  , mLatestReportId(0)
{
}

ReporterA::~ReporterA()
{
}
