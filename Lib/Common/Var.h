#pragma once

#include <QDir>
#include <QString>


QString GetVarFile(const QString& filename);
QString GetVarFileEx(const QString& filename);
QString GetVarFileWithId(const QString& filename, int id);
QString GetVarFileBin(const QString& filename);
QDir GetVarDir();
QString GetVarPath();
