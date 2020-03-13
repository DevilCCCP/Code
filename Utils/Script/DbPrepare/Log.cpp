#include <QDateTime>
#include <QDebug>

#include "Log.h"


void Log::Trace(const QString& text)
{
  mLogStream << QString("[%1] %2\n").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz"), text);
  printf("\t%s\n", text.toUtf8().constData());
}

void Log::Info(const QString& text, bool eol)
{
  if (mNewLine) {
    mLogStream << QString("[%1] %2%3").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz"), text, eol? "\n": "");
  } else {
    mLogStream << QString("%2%3").arg(text, eol? "\n": "");
  }
  printf("%s%s", text.toUtf8().constData(), eol? "\n": "");
  mNewLine = eol;
}

void Log::Warning(const QString& text)
{
  if (mNewLine) {
    mLogStream << QString("[%1] !Warning! %2\n").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz"), text);
  } else {
    mLogStream << QString(" !Warning! %2\n").arg(text);
  }
  printf("%s\n", text.toUtf8().constData());
}

Log::Log(const QString& path)
  : mLogFile(path), mLogStream(&mLogFile)
  , mNewLine(true)
{
  mLogFile.open(QFile::WriteOnly);
  mLogStream.setCodec("UTF-8");
}
