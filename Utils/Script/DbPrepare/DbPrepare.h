#pragma once

#include <QDir>

#include "Log.h"


class DbPrepare
{
  QDir    mInstDir;
  QDir    mProjDir;
  QDir    mDestDir;
  QDir    mDbGlobalDir;
  QDir    mDbLocalDir;

  QString mLocale;
  bool    mUseReport;
  bool    mUseEvent;
  bool    mUseJob;
  bool    mUseVideoArm;
  bool    mUseVa;

  Log     mLog;

public:
  int Exec();

private:
  int PrepareDir();
  int LoadSettings();
  int CreateScript(const QString& filename, bool isTable = false);
  int CreateTables(const QString& filename);
  int CreateInstall();
  void RemoveFiles(QStringList& fileList, const QStringList& removeList);
  void OrderTableFiles(const QDir& scriptDir, QStringList& fileList);

  int AddFile(QTextStream& destStream, const QDir& dir, const QString& filename);

  static QString MakeCamel(const QString& name);

public:
  DbPrepare(const QString& instPath);
};
