#pragma once

#include <QFile>
#include <QMutex>
#include <QElapsedTimer>
#include <QDateTime>

#include <Lib/Include/Common.h>

#define LOG_WARNING_ONCE(message) { static bool warned = false; if (!warned) { Log.Warning(message); warned = true; } }
#define LOG_ERROR_ONCE(message) { static bool warned = false; if (!warned) { Log.Error(message); warned = true; } }
#define LOG_FATAL_ONCE(message) { static bool warned = false; if (!warned) { Log.Fatal(message); warned = true; } }

enum ELogLevel {
  eTrace,
  eInfo,
  eStatus,
  eWarning,
  eError,
  eDebug,
  eFatal,
  eDisable
};

namespace LogPrivate
{

class Log
{
private:
  int           mMaxLogFileSize;
  QString       mFileName;     /// File name for mFile
  QFile         mFile;         /// File to write to
  ELogLevel     mLogLevel;     /// Log level
  QMutex        mAccess;       /// Access to file
  int           mStatusLength; /// Count of status chars to override them
  int           mStatusTime;   /// Time to sync status on disk
  QElapsedTimer mLastStatus;   /// Last status write
  QElapsedTimer mLastSync;     /// Last sync time
  QDate         mInitDate;     /// Last log date

private:
  static Log* GetLog() { static Log gLog; return &gLog; }

public:
  static void SetStd() { GetLog()->SetStd_(); }
  static void SetFile(const QString _FileName, int _MaxLogFileSize) { GetLog()->SetFile_(_FileName, _MaxLogFileSize); }
  static void SetFileLogging(const QString& prefix = QString("!"));
  static void SetConsoleLogging();
private:
  void SetStd_();
  void SetFile_(const QString _FileName, int _MaxLogFileSize);
  void ReleaseFileWithLock();
  void ReleaseFile();

public:
  static void SetLevel(ELogLevel _LogLevel) { GetLog()->SetLevel_(_LogLevel); }
  static void SetStatusTime(int _StatusTime) { GetLog()->SetStatusTime_(_StatusTime); }

  static void Debug(const QString& text)   { GetLog()->Out_(eDebug, text); }
  static void Trace(const QString& text)   { GetLog()->Out_(eTrace, text); }
  static void Info(const QString& text)    { GetLog()->Out_(eInfo, text); }
  static void Warning(const QString& text) { GetLog()->Out_(eWarning, text); }
  static void Error(const QString& text)   { GetLog()->Out_(eError, text); }
  static void Fatal(const QString& text, bool throwFatal = false)   { GetLog()->Out_(eFatal, text); if (throwFatal) throw FatalException(); }
  static void Status(const QString& text)  { GetLog()->Out_(eStatus, text); }

  static void File(const QString& filename, const QString& text) { GetLog()->OutFile_(filename, text); }

private:
  void SetLevel_(ELogLevel _LogLevel);
  void SetStatusTime_(int _StatusTime);

  void Out_(const ELogLevel& level, const QString& text);
  void OutFile_(const QString& filename, const QString& text);

private:
  void Sync();
  void Rewind();

private:
  Log();
  ~Log();
};
}

static LogPrivate::Log* gNullLog = nullptr;
static LogPrivate::Log& Log = *gNullLog;

inline void LogUnwarningFunction()
{
  Log.Info("'Log' defined but not used [-Wunused-variable]");
}
