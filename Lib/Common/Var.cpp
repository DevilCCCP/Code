#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

#include "Var.h"


QString GetVarFile(const QString& filename)
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Var");
  dir.cd("Var");
  return dir.absoluteFilePath(QString("%1.ini").arg(filename));
}

QString GetVarFileEx(const QString& filename)
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Var");
  dir.cd("Var");
  return dir.absoluteFilePath(filename);
}

QString GetVarFileWithId(const QString& filename, int id)
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Var");
  dir.cd("Var");
  return dir.absoluteFilePath(QString("%1_%2.ini").arg(id, 6, 10, QChar('0')).arg(filename));
}

QString GetVarFileBin(const QString& filename)
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Var");
  dir.cd("Var");
  return dir.absoluteFilePath(QString("%1.bin").arg(filename));
}

QString GetVarPath()
{
  return GetVarDir().absolutePath();
}


QDir GetVarDir()
{
  QString path = QCoreApplication::applicationDirPath();
  QDir dir(path);
  dir.mkdir("Var");
  dir.cd("Var");
  return dir;
}
