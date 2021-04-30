#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

#include <Lib/Log/Log.h>
#include <Lib/Common/Var.h>
#include <Lib/Crypto/InnerCrypt.h>

#include "Db.h"
#include "ObjectType.h"
#include "ObjectSettings.h"
#include "ObjectSettingsType.h"
#include "ObjectState.h"
#include "ObjectStateHours.h"
#include "ObjectState2.h"
#include "ObjectLog.h"
#include "ObjectLogInfo.h"
#include "Files.h"
#include "Event.h"
#include "VaStat.h"
#include "VaStatDays.h"
#include "VaStatHours.h"
#include "VaStatType.h"
#include "Report.h"
#include "ReportSend.h"
#include "ReportFilesMap.h"
#include "ArmMonitors.h"
#include "MonitorLayouts.h"
#include "AmlCamMap.h"
#include "Variables.h"


#define UseDbTableOne(XXX) PROPERTY_SGET_DEMAND_INST(Db, XXX##Table, *this)
UseDbTables
#undef UseDbTableOne

#define UseDbViewOne(XXX) PROPERTY_SGET_DEMAND_INST(Db, XXX##View, *this)
UseDbViews
#undef UseDbViewOne

static QString GetNextConnectionName()
{
  static QMutex gMutex;
  QMutexLocker lock(&gMutex);
  static int gDbNum = 13;
  return QString::number(++gDbNum);
}

QString GetPreparedQueryText(const QSqlQuery& query)
{
  QString sql = query.executedQuery();
  int nbBindValues = query.boundValues().size();

  for (int i = 0, j = 0; j < nbBindValues;)
  {
    int s = sql.indexOf(QLatin1Char('\''), i);
    i = sql.indexOf(QLatin1Char('?'), i);
    if (i < 1) {
      break;
    }

    if (s < i && s > 0) {
      i = sql.indexOf(QLatin1Char('\''), s + 1) + 1;
      if (i < 2) {
        break;
      }
    } else {
      const QVariant &var = query.boundValue(j);
      QSqlField field(QLatin1String(""), var.type());
      if (var.isNull()) {
        field.clear();
      } else {
        field.setValue(var);
      }
      QString formatV = query.driver()->formatValue(field);
      if (formatV.size() > 200) {
        formatV = QString("%1..<cut>..%2").arg(formatV.left(40)).arg(formatV.right(40));
      }
      sql.replace(i, 1, formatV);
      i += formatV.length();
      ++j;
    }
  }

  return sql;
}


QVariant Db::ToKey(int id)
{
  return (id)? QVariant(id): QVariant();
}

QVariant Db::ToKey(const qint64& id)
{
  return (id)? QVariant(id): QVariant();
}

QVariant Db::ToPoint(const QPoint& p)
{
  return QString("(%1,%2)").arg(QString::number(p.x()), QString::number(p.y()));
}

QPoint Db::FromPoint(const QVariant& pv)
{
  QByteArray pText = pv.toByteArray();
  QByteArray pX;
  QByteArray pY;
  const char* pdata = pText.constData();
  int psize = pText.size();
  int state = 0;
  for (int i = 0; i < psize; i++, pdata++) {
    switch (*pdata) {
    case '(': state = -1; break;
    case ')': state = 0; break;
    case ',': state = 1; break;
    case ' ': break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case '-':
      switch (state) {
      case -1: pX.append(*pdata); break;
      case  1: pY.append(*pdata); break;
      }
      break;
    }
  }
  return QPoint(pX.toInt(), pY.toInt());
}

QString Db::ArrayToString(const QStringList& list)
{
  QString text;
  for (auto itr = list.begin(); itr != list.end(); itr++) {
    const QString& value = *itr;
    int state = 0;
    for (auto itr = value.begin(); itr != value.end(); itr++) {
      switch (itr->toLatin1()) {
      case '~':
        text.append(*itr);
        state = 1;
        break;
      case '|':
        if (state) {
          text.append("|~~||");
        } else {
          text.append("~||");
        }
        state = 0;
        break;
      default:
        text.append(*itr);
        state = 0;
        break;
      }
    }
    if (state) {
      text.append("|~");
    }
    text.append("|");
  }
  text.truncate(text.size() - 1);
  return text;
}

QStringList Db::StringToArray(const QString& text)
{
  QStringList list;
  QString next;
  int state = 0;
  for (auto itr = text.begin(); itr != text.end(); itr++) {
    switch (itr->toLatin1()) {
    case '~':
      state = 1;
      break;
    case '|':
      if (state) {
        itr++;
        if (itr == text.end()) {
          Log.Warning(QString("Parse string to array fail (text: '%1')").arg(text));
          return list;
        }
        next.append(*itr);
      } else {
        list.append(next);
        next = QString();
      }
      break;
    default:
      next.append(*itr);
      break;
    }
  }
  return list;
}

void Db::MoveToThread(QThread* thread) const
{
  if (!thread) {
    thread = QThread::currentThread();
  } else {
    Log.Trace(QString("Db '%1' move to thread").arg(mDbConnectionName));
  }
  mDbThread = thread;
}

void Db::SetTrace(bool _TraceAll) const
{
  mTraceAll = _TraceAll;
}

bool Db::Exec(const QString &query, bool warn) const
{
  ValidateThread();

  auto q = mDb->exec(query);
  if (q.lastError().isValid()) {
    if (warn) {
      Log.Warning(QString("Exec error: %1 (error: '%2', code: %3)").arg(query).arg(q.lastError().text()).arg(q.lastError().number()));
    }
    return false;
  }
  return true;
}

bool Db::OpenFromFile(const QString& filename)
{
  QFile file(GetVarFile(filename));
  if (!file.open(QIODevice::ReadOnly)) {
    Log.Fatal(QString("No connection file '%1'").arg(filename));
    return false;
  }

  QTextStream in(&file);
  in.setCodec("UTF-8");
  QString text;
  do {
    text = in.readLine().trimmed();
  } while (text.startsWith('#'));
  file.close();

  QStringList info = text.split("::");
  if (info.count() < 5) {
    Log.Fatal(QString("Invalid connection file '%1'").arg(filename));
    return false;
  } else {
    return Open(info[0], info[1], info[2], info[3], info[4]);
  }
}

bool Db::SaveConnectionFile(const QString &filename, const QString &_Server, const QString &_Port, const QString &_Database, const QString &_User, const QString &_Password)
{
  QString server = (_Server.size() == 0)? mDb->hostName(): _Server;
  QString port = (_Port.size() == 0)? QString::number(mDb->port()): _Port;
  QString database = (_Database.size() == 0)? mDb->databaseName(): _Database;
  QString user = (_User.size() == 0)? mDb->userName(): _User;
  QString password = (_Password.size() == 0)? mDb->password(): _Password;

  QString filePath = GetVarFile(filename);
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    Log.Fatal(QString("Can't create connection file '%1'").arg(filePath));
    return false;
  }

  if (!mInnerCrypt) {
    mInnerCrypt.reset(new InnerCrypt());
  }
  QString text = QString("%1::%2::%3::%4::%5").arg(server).arg(port).arg(database).arg(user).arg(QString::fromLatin1(mInnerCrypt->Encrypt(password.toUtf8()).toBase64()));
  QTextStream out(&file);
  out.setCodec("UTF-8");
  out << text;
  file.close();

  QDir appDir(QCoreApplication::applicationDirPath());
  if (appDir.dirName() == "debug") {
    QString fileCopy = filePath;
    fileCopy.replace("debug", "release");
    QFile fileEx(fileCopy);
    if (fileEx.open(QIODevice::WriteOnly)) {
      QTextStream outEx(&fileEx);
      outEx.setCodec("UTF-8");
      outEx << text;
      fileEx.close();
    } else {
      Log.Warning("SaveConnectionFile: dublicate release save fail");
    }
  } else if (appDir.dirName() == "release") {
    QString fileCopy = filePath;
    fileCopy.replace("release", "debug");
    QFile fileEx(fileCopy);
    if (fileEx.open(QIODevice::WriteOnly)) {
      QTextStream outEx(&fileEx);
      outEx.setCodec("UTF-8");
      outEx << text;
      fileEx.close();
    } else {
      Log.Warning("SaveConnectionFile: dublicate debug save fail");
    }
  }
  return true;
}

bool Db::Open(const QString &_Server, const QString &_Port, const QString &_Database, const QString &_User, const QString &_Password)
{
  mDb->setDatabaseName(_Database);
  if (!_Server.isEmpty()) {
    mDb->setHostName(_Server);
  }
  if (!_Port.isEmpty()) {
    mDb->setPort(_Port.toInt());
  }
  mDb->setUserName(_User);
  if (!mInnerCrypt) {
    mInnerCrypt.reset(new InnerCrypt());
  }
  QByteArray pwdEnc = QByteArray::fromBase64(_Password.toLatin1());
  QByteArray pwd = mInnerCrypt->Decrypt(pwdEnc);
  if (pwd.isEmpty()) {
    pwd = _Password.toUtf8();
    LOG_WARNING_ONCE(QString("Decrypt connection password fail, use as unencrypted (error: '%1')").arg(mInnerCrypt->ErrorText()));
  }
  mDb->setPassword(QString::fromUtf8(pwd));
  return true;
}

bool Db::Connect() const
{
  ValidateThread();

  if (mDb->isOpen()) {
    return true;
  }

  static int gLastError = 0;
  if (mDb->open()) {
    if (gLastError) {
      gLastError = 0;
      Log.Info(QString("Connection DB ok"));
    }
    return true;
  } else {
    int error = mDb->lastError().number();
    if (error != gLastError) {
      Log.Warning(QString("Connection DB fail (code: %1): %2").arg(error).arg(mDb->lastError().text()));
      gLastError = error;
    }
    return false;
  }
}

void Db::Close() const
{
  if (mDb->isOpen()) {
    mDb->close();
  }
}

bool Db::IsConnected() const
{
  return mDb->isOpen();
}

const Version& Db::GetVersion() const
{
  if (mVersion.getMajorVersion() > 0) {
    return mVersion;
  }

  auto q = MakeQuery();
  q->prepare("SELECT current_setting('server_version')");
  QVariant v = ExecuteScalar(q);
  mVersion.LoadFromString(v.toString());
  return mVersion;
}

bool Db::ExecuteBatch(QueryS& query) const
{
  ValidateThread();

  if (!mDb->isOpen() && !mDb->open()) {
    return false;
  }
  Q_ASSERT(query);

//#ifndef QT_NO_DEBUG
//  if (mTraceAll) {
//    Log.Trace(GetPreparedQueryText(*query));
//  }
//#endif
  if (!mDb->transaction()) {
    return false;
  }
  if (query->execBatch() && mDb->commit()) {
    return true;
  }
  query->clear();
  if (!mDb->rollback()) {
    Log.Warning(QString("query rollback fail"));
  }
  Log.Warning(QString("'%1' fail (error: '%2', code: %3)").arg(GetPreparedQueryText(*query)).arg(query->lastError().text()).arg(query->lastError().number()));
  return false;
}

bool Db::ExecuteQuery(QueryS& query) const
{
  ValidateThread();

  if (!mDb->isOpen() && !mDb->open()) {
    return false;
  }
  Q_ASSERT(query);

#ifndef QT_NO_DEBUG
  if (mTraceAll) {
    Log.Trace(GetPreparedQueryText(*query));
  }
#endif
  if (query->exec()) {
    return true;
  }

  do {
    if (!query->lastError().isValid()) {
      Log.Warning(QString("Query fail without error, closing connection"));
      break;
    } else if (query->lastError().type() == QSqlError::ConnectionError) {
      Log.Warning(QString("Query fail with connection error, closing connection"));
      break;
    } else if (query->lastError().type() == QSqlError::UnknownError) {
      Log.Warning(QString("Query fail with unknown error, closing connection"));
      break;
    }

    if (query->lastError().type() == QSqlError::StatementError) {
      Log.Warning(QString("'%1' fail (error: '%2')")
                  .arg(GetPreparedQueryText(*query)).arg(query->lastError().text()));
    } else {
      Log.Warning(QString("Query fail (error: '%1', code: %2)")
                  .arg(query->lastError().text()).arg(query->lastError().type()));
    }
    return false;

  } while (false);

  mDb->close();
  return false;
}

QVariant Db::ExecuteScalar(QueryS& query) const
{
  Q_ASSERT(query.data() != NULL);
  QVariant result = QVariant();
  if (ExecuteQuery(query) && query->next()) {
    result = query->value(0);
  }
  return result;
}

bool Db::ExecuteNonQuery(QueryS& query) const
{
  return ExecuteQuery(query);
}

DbTransactionS Db::BeginTransaction() const
{
  ValidateThread();

  if (mDb->transaction()) {
    return DbTransactionS(new DbTransaction(*this));
  }
  return DbTransactionS();
}

bool Db::CommitTransaction() const
{
  return mDb->commit();
}

bool Db::RollbackTransaction() const
{
  return mDb->rollback();
}

void Db::RegisterDb(const QString& dbType)
{
  mDbConnectionName = GetNextConnectionName();
  mDb.reset(new QSqlDatabase(QSqlDatabase::addDatabase(dbType, mDbConnectionName)));
}

void Db::ValidateThread() const
{
  QThread* thread = QThread::currentThread();
  if (mDbThread != thread) {
    if (!mThreadWarnings.contains(thread)) {
      Log.Error(QString("Using Db '%1' from extern thread").arg(mDbConnectionName));
      mThreadWarnings.insert(thread);
    }
  }
}


Db::Db(const char* _DbType, bool _TraceAll)
  : mTraceAll(_TraceAll)
  , mDbThread(nullptr)
{
  QString dbType(_DbType);
  RegisterDb(dbType);
  MoveToThread();
}

Db::Db(bool _TraceAll)
  : mTraceAll(_TraceAll)
  , mDbThread(nullptr)
{
  static QString gPgDbType("QPSQL");
  RegisterDb(gPgDbType);
  MoveToThread();
}

Db::~Db()
{
  mDb->close();
  mDb.reset();
  QSqlDatabase::removeDatabase(mDbConnectionName);
}

