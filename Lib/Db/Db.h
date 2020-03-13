#pragma once

#include <QtSql/QtSql>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QStringBuilder>
#include <QSet>

#include <Lib/Include/Common.h>
#include <Lib/Common/Version.h>

#include "DbDef.h"
#include "DbInline.h"
#include "DbTransaction.h"


typedef QSharedPointer<QSqlQuery> QueryS;

DefineClassS(Db);
DefineClassS(InnerCrypt);
typedef QScopedPointer<QSqlDatabase> QSqlDatabaseP;

class Db
{
  QSqlDatabaseP           mDb;
  QString                 mDbConnectionName;
  InnerCryptS             mInnerCrypt;
  mutable bool            mTraceAll;

  mutable QThread*        mDbThread;
  mutable QSet<QThread*>  mThreadWarnings;

#define UseDbTableOne(XXX) PROPERTY_SGET_DEMAND(XXX##Table);
  UseDbTables
#undef UseDbTableOne

#define UseDbViewOne(XXX) PROPERTY_SGET_DEMAND(XXX##View);
  UseDbViews
#undef UseDbViewOne

  mutable Version         mVersion;

public:
  QString Host() const { return mDb->hostName(); }
  int Port() const { return mDb->port(); }

public:
  static QVariant ToKey(int id);
  static QVariant ToKey(const qint64& id);
  static QVariant ToPoint(const QPoint& p);
  static QPoint FromPoint(const QVariant& pv);
  static QString ArrayToString(const QStringList& list);
  static QStringList StringToArray(const QString& text);

  void MoveToThread(QThread* thread = nullptr) const;

  void SetTrace(bool _TraceAll = true) const;
  bool Exec(const QString& query, bool warn = true) const;
  inline QueryS MakeQuery(bool forwardOnly = true) const { QueryS q(new QSqlQuery(*mDb)); q->setForwardOnly(forwardOnly); return q; }

  bool OpenDefault() { return OpenFromFile("connection"); }
  bool OpenLocal() { return OpenFromFile("connection_local"); }
  bool OpenFromFile(const QString& filename);
  bool SaveConnectionFile(const QString &filename, const QString& _Server, const QString& _Port, const QString& _Database, const QString& _User, const QString& _Password);
  bool Open(const QString& _Server, const QString& _Port, const QString& _Database, const QString& _User, const QString& _Password);
  bool Connect() const;
  void Close() const;
  bool IsConnected() const;

  const Version& GetVersion() const;

  bool ExecuteBatch(QueryS& query) const;
  bool ExecuteQuery(QueryS& query) const;
  QVariant ExecuteScalar(QueryS& query) const;
  bool ExecuteNonQuery(QueryS& query) const;

  DbTransactionS BeginTransaction() const;
  bool CommitTransaction() const;
  bool RollbackTransaction() const;

private:
  void RegisterDb(const QString& dbType);
  void ValidateThread() const;

public:
  Db(const char* _DbType, bool _TraceAll = true);
  Db(bool _TraceAll = true);
  ~Db();
};

