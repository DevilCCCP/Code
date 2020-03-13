#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#include "DirCopy.h"


bool CopyDirRecursively(const QString& source, const QString& dest)
{
  QDir dirSource(source);
  QDir dirDest(dest);
  if (!dirSource.isReadable()) {
    return false;
  }
  if (!dirDest.mkdir(dest)) {
    return false;
  }

  QDirIterator itr(dirSource.absolutePath(), QStringList() << "*", QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                   , QDirIterator::Subdirectories);
  while (itr.hasNext()) {
    QString sourceFile = itr.next();
    QString filePath = dirSource.relativeFilePath(sourceFile);
    QFileInfo info(sourceFile);
    if (info.isDir()) {
      if (!dirDest.mkdir(filePath)) {
        return false;
      }
    } else if (info.isFile()) {
      if (!QFile::copy(sourceFile, dirDest.absoluteFilePath(filePath))) {
        return false;
      }
    }
  }
  return true;
}
