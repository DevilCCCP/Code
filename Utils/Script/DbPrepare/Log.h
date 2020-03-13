#pragma once

#include <QFile>
#include <QTextStream>


class Log
{
  QFile       mLogFile;
  QTextStream mLogStream;

  bool        mNewLine;

public:
  void Trace(const QString& text);
  void Info(const QString& text, bool eol = true);
  void Warning(const QString& text);

public:
  Log(const QString& path);
};
