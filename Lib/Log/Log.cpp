#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <qsystemdetection.h>
#ifdef Q_OS_WIN32
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif

#include "Log.h"

namespace LogPrivate
{

const qint64 kSyncTime = 0;
const qint64 kStatusTime = 5 * 60 * 1000;
const int    kMaxLogFileSize = 10 * 1024 * 1024;

void Log::SetFileLogging(const QString& prefix)
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Log");
  dir.cd("Log");
  QFileInfo appFile(QCoreApplication::applicationFilePath());
  SetFile(dir.absoluteFilePath(QString("%1%2.log").arg(prefix).arg(appFile.completeBaseName())), kMaxLogFileSize);
  QDir::setCurrent(path);
}

void Log::SetConsoleLogging()
{
#ifdef Q_OS_WIN32
/// http://www.halcyon.com/~ast/dload/guicon.htm
  int hConHandle;
  long lStdHandle;
  CONSOLE_SCREEN_BUFFER_INFO coninfo;
  FILE *fp;

// allocate a console for this app
  AllocConsole();

// set the screen buffer to be big enough to let us scroll text
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
  coninfo.dwSize.Y = 4000;
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

// redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen( hConHandle, "w" );

  *stdout = *fp;

  setvbuf( stdout, NULL, _IONBF, 0 );

// redirect unbuffered STDIN to the console

  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen( hConHandle, "r" );
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

  fp = _fdopen( hConHandle, "w" );

  *stderr = *fp;

  setvbuf( stderr, NULL, _IONBF, 0 );

// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
// point to console as well
  std::ios::sync_with_stdio();
#endif
}

void Log::SetStd_()
{
  mAccess.lock();
  ReleaseFile();

  mFileName.clear();
  mFile.open(stdout, QIODevice::WriteOnly | QIODevice::Text);
  mAccess.unlock();
}

void Log::SetFile_(const QString _FileName, int _MaxLogFileSize)
{
  mAccess.lock();
  ReleaseFile();

  mMaxLogFileSize = _MaxLogFileSize;

  QFileInfo fi(_FileName);
  QDir dir = fi.absoluteDir();
  dir.mkpath(dir.absolutePath());
  mFile.setFileName(_FileName);
  if (mFile.open(QIODevice::Append | QIODevice::Text)) {
    mFileName = _FileName;
    mFile.write("---=== Open new session ===---\n");
  }
  mAccess.unlock();
}

void Log::ReleaseFileWithLock()
{
  mAccess.lock();

  ReleaseFile();
  mAccess.unlock();
}

void Log::ReleaseFile()
{
  if (mFile.isOpen()) {
    mFile.close();
    mFileName.clear();
  }
}

void Log::SetLevel_(ELogLevel _LogLevel)
{
  mLogLevel = _LogLevel;
}

void Log::SetStatusTime_(int _StatusTime)
{
  mStatusTime = _StatusTime;
}

inline int QStringLenEx(const QString& allText)
{
  int len = 0;
  for (int i = 0; i < allText.size(); i++) {
    len += (allText[i] == '\t')? 8: 1;
  }
  return len;
}

void Log::Out_(const ELogLevel& level, const QString& text)
{
  QString allText;
  if (level >= mLogLevel && mFile.isOpen()) {
    mAccess.lock();
    QDateTime now = QDateTime::currentDateTime();
    const char* levelText;
    switch (level) {
    case eStatus:  levelText = "Status "; break;
    case eDebug:   levelText = "Debug  "; break;
    case eTrace:   levelText = "Trace  "; break;
    case eInfo:    levelText = "Info   "; break;
    case eWarning: levelText = "Warning"; break;
    case eError:   levelText = "Error  "; break;
    case eFatal:   levelText = "Fatal  "; break;
    default:       levelText = "       "; break;
    }

    if (mFileName.isEmpty()) { // console
      if (level != eStatus) {
        if (mStatusLength) {
          allText += QString("[%1:%2:%3.%4 %5] %6: %7")
            .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
            .arg((qintptr)QThread::currentThreadId(), 8).arg(levelText).arg(text);
          int oldTextLen = mStatusLength;
          mStatusLength = QStringLenEx(allText);
          if (oldTextLen - mStatusLength) {
            allText += QString(oldTextLen - mStatusLength, ' ');
          }
          allText += '\n';
        } else {
          allText += QString("[%1:%2:%3.%4 %5] %6: %7\n")
            .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
            .arg((qintptr)QThread::currentThreadId(), 8).arg(levelText).arg(text);
        }
        mStatusLength = 0;
      } else {
        if (mStatusLength) {
          allText += QString("[%1:%2:%3.%4 %5] %6: %7")
            .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
            .arg((qintptr)QThread::currentThreadId(), 8).arg(levelText).arg(text);
          int oldTextLen = mStatusLength;
          mStatusLength = QStringLenEx(allText);
          if (oldTextLen - mStatusLength) {
            allText += QString(oldTextLen - mStatusLength, ' ');
          }
          allText += '\r';
        } else {
          allText += QString("[%1:%2:%3.%4 %5] %6: %7\r")
            .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
            .arg((qintptr)QThread::currentThreadId(), 8).arg(levelText).arg(text);
          mStatusLength = QStringLenEx(allText);
        }
      }
      mFile.write(allText.toUtf8());
      Sync();
    } else { // file
      if (mInitDate != now.date()) {
        mInitDate = now.date();
        QString newDate = QString("[Current date: %1.%2.%3] PID: %4\n")
          .arg(now.date().day(), 2, 10, QChar('0')).arg(now.date().month(), 2, 10, QChar('0')).arg(now.date().year(), 4, 10, QChar('0'))
          .arg(QCoreApplication::applicationPid());
        mFile.write(newDate.toUtf8());
        Sync();
      }

      allText += QString("[%1:%2:%3.%4 %5] %6: %7\n")
        .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
        .arg((qintptr)QThread::currentThreadId(), 8).arg(levelText).arg(text);
      if (level != eStatus) {
        mFile.write(allText.toUtf8());
        Sync();
      } else {
        if (mLastStatus.elapsed() > kStatusTime) {
          mLastStatus.restart();
          mFile.write(allText.toUtf8());
          Sync();
        }
      }
    }
    mAccess.unlock();
  }
}

void Log::OutFile_(const QString& filename, const QString& text)
{
  QFile file(filename);
  if (file.open(QFile::Append)) {
    file.write((text + '\n').toUtf8());
  }
}

void Log::Sync()
{
  if (!mFileName.isEmpty()) {
    if (mLastSync.elapsed() >= kSyncTime) {
      mLastSync.restart();
      mFile.flush();
    }
    if (mMaxLogFileSize && mFile.size() > mMaxLogFileSize) {
      Rewind();
    }
  } else {
    mFile.flush();
  }
}

void Log::Rewind()
{
  QString oldFile = mFileName + ".old";
  if (!QFile::remove(oldFile)) {
    mFile.write("<<< remove old file fail >>>\n");
  }

  mFile.close();

  if (QFile::rename(mFileName, oldFile)) {
    mFile.open(QIODevice::Append | QIODevice::Text);
  } else {
    mFile.open(QIODevice::ReadOnly | QIODevice::Text);

    mFile.seek(mFile.size() - mMaxLogFileSize/4);
    QByteArray buf = mFile.read(mMaxLogFileSize/4);
    mFile.close();

    mFile.open(QIODevice::WriteOnly | QIODevice::Text);
    mFile.write(buf);
    QDateTime now = QDateTime::currentDateTime();
    QString rwText = QString("---=== rewind at %1.%2.%3 %4:%5:%6.%7 by %8 ===---\n")
      .arg(now.date().day(), 2).arg(now.date().month(), 2).arg(now.date().year(), 2)
      .arg(now.time().hour(), 2).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0')).arg(now.time().msec(), 3, 10, QChar('0'))
      .arg(QCoreApplication::applicationPid());
    mFile.write(rwText.toLatin1());
  }
}


Log::Log()
  : mMaxLogFileSize(0)
#ifdef QT_NO_DEBUG
  , mLogLevel(eInfo)
#else
  , mLogLevel(eTrace)
#endif
  , mStatusLength(0), mStatusTime(kStatusTime)
{
  mFile.open(stdout, QIODevice::WriteOnly | QIODevice::Text);
  mInitDate = QDate::currentDate().addDays(-1);
  mLastStatus.start();
  mLastSync.start();
}


Log::~Log()
{
  ReleaseFile();
}
} // namespace
