#include <QDir>
#include <QSettings>
#include <QRegExp>
#include <QDebug>

#include "DbPrepare.h"


const QStringList kEventTables = QStringList()
    << "event_type" << "event" << "event_log" << "event_log_hours" << "event_log_triggers"
    << "event_stat" << "event_stat_hours" << "event_stat_triggers" << "event_init" << "event_stat_init";
const QStringList kReportTables = QStringList()
    << "report" << "report_files" << "report_send";
const QStringList kJobTables = QStringList()
    << "job" << "job_action" << "job_data" << "job_cancel" << "job_done" << "job_take" << "job_init";
const QStringList kVideoArmTables = QStringList()
    << "arm_monitors" << "arm_monitor_layouts" << "arm_monitor_lay_cameras";
const QStringList kVaTables = QStringList()
    << "va_stat" << "va_stat_type" << "va_stat_hours" << "va_stat_days";

int DbPrepare::Exec()
{
  if (int ret = PrepareDir()) {
    return ret;
  }
  if (int ret = LoadSettings()) {
    return ret;
  }

  mLog.Info("Building sql scripts");
  if (int ret = CreateScript("Functions")) {
    mLog.Warning("Failed :(");
    return ret;
  }
  if (int ret = CreateTables("Tables")) {
    mLog.Warning("Failed :(");
    return ret;
  }
  if (int ret = CreateScript("View")) {
    mLog.Warning("Failed :(");
    return ret;
  }
  if (int ret = CreateScript("Write")) {
    mLog.Warning("Failed :(");
    return ret;
  }
  if (int ret = CreateScript("Update")) {
    mLog.Warning("Failed :(");
    return ret;
  }

  if (int ret = CreateInstall()) {
    mLog.Warning("Failed :(");
    return ret;
  }

  mLog.Info("Done");
  return 0;
}

int DbPrepare::PrepareDir()
{
  mDestDir = mInstDir;
  if (!mDestDir.cd("Db")) {
    qWarning() << "Install Db dir doesn't exists";
    return 1;
  }
  mProjDir = mInstDir;
//  qDebug() << "Running in" << projDir.absolutePath();

  forever {
    if (mProjDir.exists("Local.pri")) {
      break;
    }
    if (!mProjDir.cdUp()) {
      qWarning() << "Project directory not found seeking up from" << mInstDir.absolutePath();
      return 1;
    }
  }

  mLog.Trace(QString("Project is '%1'").arg(mProjDir.absolutePath()));
  if (!mInstDir.exists("Db.ini")) {
    qWarning() << "Db.ini not exists, using defaults";
  }

  mDbLocalDir = mProjDir;
  if (!mDbLocalDir.cd("Db")) {
    qWarning() << "Project's local Db dir doesn't exists";
    return 1;
  }

  mDbGlobalDir = mProjDir;
  mDbGlobalDir.cdUp();
  if (!mDbGlobalDir.cd("Misc/Db")) {
    qWarning() << "Project's global Db dir doesn't exists";
    return 1;
  }
  return 0;
}

int DbPrepare::LoadSettings()
{
  QSettings settings(mInstDir.absoluteFilePath("Db.ini"), QSettings::IniFormat);

  Log mLog(mInstDir.absoluteFilePath("DbPrepare.log"));

  mLocale = settings.value("Locale", "Ru").toString();
  mExtantion = settings.value("Extantion", "").toString();
  mUseReport = settings.value("Report", false).toBool();
  mUseEvent = settings.value("Event", false).toBool();
  mUseJob = settings.value("Job", false).toBool();
  mUseVideoArm = settings.value("VideoArm", false).toBool();
  mUseVa = settings.value("Va", false).toBool();

  mLog.Trace(QString("Locale: '%1'").arg(mLocale));
  mLog.Trace(QString("Extantion: '%1'").arg(mExtantion));
  mLog.Trace(QString("Report: %1").arg(mUseReport? "true": "false"));
  mLog.Trace(QString("Event: %1").arg(mUseEvent? "true": "false"));
  mLog.Trace(QString("Job: %1").arg(mUseJob? "true": "false"));
  mLog.Trace(QString("VideoArm: %1").arg(mUseVideoArm? "true": "false"));
  mLog.Trace(QString("Va: %1").arg(mUseVa? "true": "false"));
  return 0;
}

int DbPrepare::CreateScript(const QString& filename, bool isTable)
{
  mLog.Info(QString("%1 .. ").arg(filename), false);
  QFile scriptFile(mDestDir.absoluteFilePath(filename + ".sql"));
  if (!scriptFile.open(QFile::WriteOnly)) {
    mLog.Info(QString("%1.sql open fail").arg(filename));
    return 1;
  }
  QTextStream fileStream(&scriptFile);
  fileStream.setCodec("UTF-8");
  fileStream << "-- AUTO generated script --\n\n";

  QDir scriptDir = (mDbGlobalDir);
  if (!scriptDir.cd(filename) && !scriptDir.cd(filename + mLocale)) {
    return 1;
  }

  QStringList fileList = scriptDir.entryList(QDir::Files, QDir::Name);
  RemoveAllFiles(fileList);

  if (isTable) {
    OrderTableFiles(scriptDir, fileList);
  }

  foreach (const QString& filename, fileList) {
    if (int ret = AddFile(fileStream, scriptDir, filename)) {
      return ret;
    }
  }

  if (!mExtantion.isEmpty()) {
    scriptDir = (mDbGlobalDir);
    if (scriptDir.cd(filename + mExtantion)) {
      QStringList fileList = scriptDir.entryList(QDir::Files, QDir::Name);
      RemoveAllFiles(fileList);

      if (isTable) {
        OrderTableFiles(scriptDir, fileList);
      }

      foreach (const QString& filename, fileList) {
        if (int ret = AddFile(fileStream, scriptDir, filename)) {
          return ret;
        }
      }
    }
  }

  scriptDir = (mDbLocalDir);
  fileList.clear();
  if (scriptDir.cd(filename)) {
    fileList = scriptDir.entryList(QDir::Files, QDir::Name);
  }

  OrderTableFiles(scriptDir, fileList);

  foreach (const QString& filename, fileList) {
    if (int ret = AddFile(fileStream, scriptDir, filename)) {
      return ret;
    }
  }

  fileStream << "\n";
  mLog.Info("Ok");
  return 0;
}

int DbPrepare::CreateTables(const QString& filename)
{
  return CreateScript(filename, true);
}

int DbPrepare::CreateInstall()
{
  QFile fullInst(mDestDir.absoluteFilePath("Install.sql"));
  if (!fullInst.open(QFile::WriteOnly)) {
    mLog.Info("Install.sql open fail");
    return 1;
  }
  QTextStream fileStream(&fullInst);
  fileStream.setCodec("UTF-8");
  fileStream << "-- AUTO generated Install script --\n\nBEGIN TRANSACTION;\n\n";

  QStringList filenameList = QStringList() << "Functions.sql" << "Tables.sql" << "View.sql" << "Write.sql" << "Update.sql";
  foreach (const QString& filename, filenameList) {
    if (int ret = AddFile(fileStream, mDestDir, filename)) {
      return ret;
    }
  }

  fileStream << "-- End of Install script --\nEND TRANSACTION;\n";
  return 0;
}

void DbPrepare::RemoveAllFiles(QStringList& fileList)
{
  if (!mUseReport) {
    RemoveFiles(fileList, kReportTables);
  }
  if (!mUseEvent) {
    RemoveFiles(fileList, kEventTables);
  }
  if (!mUseJob) {
    RemoveFiles(fileList, kJobTables);
  }
  if (!mUseVideoArm) {
    RemoveFiles(fileList, kVideoArmTables);
  }
  if (!mUseVa) {
    RemoveFiles(fileList, kVaTables);
  }
}

void DbPrepare::RemoveFiles(QStringList& fileList, const QStringList& removeList)
{
  foreach (const QString& tableName, removeList) {
    fileList.removeAll(QString("%1.sql").arg(tableName));
  }
}

void DbPrepare::OrderTableFiles(const QDir& scriptDir, QStringList& fileList)
{
  QRegExp refRegexp("REFERENCES\\s+(\\w+)");
  QRegExp ref2Regexp("CREATE\\s+TRIGGER.*\\s+ON\\s+(\\w+)");
  QRegExp ref3Regexp("--\\s+DEPENDS\\s+(\\w+)");
  QMap<QString, QSet<QString> > referenceMap;
  foreach (const QString& filename, fileList) {
    QFile file(scriptDir.absoluteFilePath(filename));
    if (!file.open(QFile::ReadOnly)) {
      mLog.Warning(QString("Open table file '%1' fail").arg(filename));
      continue;
    }
    QTextStream fileSubstream(&file);
    fileSubstream.setCodec("UTF-8");
    while (!fileSubstream.atEnd()) {
      QString line = fileSubstream.readLine();
      QString refTable;
      if (refRegexp.indexIn(line) >= 0) {
        refTable = refRegexp.cap(1);
      } else if (ref2Regexp.indexIn(line) >= 0) {
        refTable = ref2Regexp.cap(1);
      } else if (ref3Regexp.indexIn(line) >= 0) {
        refTable = ref3Regexp.cap(1);
      }
      if (!refTable.isEmpty()) {
        QString refTable1 = QString("%1.sql").arg(refTable);
        QString refTable2 = QString("%1.sql").arg(MakeCamel(refTable));
        if (refTable2 != filename && fileList.contains(refTable2)) {
          referenceMap[filename].insert(refTable2);
        }
        if (refTable1 != filename && fileList.contains(refTable1)) {
          referenceMap[filename].insert(refTable1);
        }
      }
    }
  }

  QStringList nextList;
  nextList.swap(fileList);

  while (!nextList.isEmpty()) {
    int initSize = nextList.size();

    for (int i = 0; i < nextList.size(); i++) {
      const QString& filename = nextList.at(i);
      if (referenceMap[filename].isEmpty()) {
        for (auto itr = referenceMap.begin(); itr != referenceMap.end(); itr++) {
          itr.value().remove(filename);
        }
        fileList.append(filename);
        nextList.removeAt(i);
        break;
      }
    }

    if (nextList.size() >= initSize) {
      mLog.Warning(QString("Order tables fail ('%1')").arg(nextList.join(',')));
      fileList.append(nextList);
      break;
    }
  }
  qInfo() << fileList;
}

int DbPrepare::AddFile(QTextStream& destStream, const QDir& dir, const QString& filename)
{
  QFile halfInst(dir.absoluteFilePath(filename));
  if (!halfInst.open(QFile::ReadOnly)) {
    mLog.Info(QString("%1 open fail").arg(filename));
    return 1;
  }
  QTextStream fileSubstream(&halfInst);
  fileSubstream.setCodec("UTF-8");
  destStream << fileSubstream.readAll();
  return 0;
}

QString DbPrepare::MakeCamel(const QString& name)
{
  QStringList wordList = name.split("_", QString::SkipEmptyParts);
  for (auto itr = wordList.begin(); itr != wordList.end(); itr++) {
    QString* word = &*itr;
    QChar* ch = &*word->begin();
    *ch = ch->toUpper();
  }
  return wordList.join("");
}

DbPrepare::DbPrepare(const QString& instPath)
  : mInstDir(instPath), mLog(mInstDir.absoluteFilePath("DbPrepare.log"))
{
}
