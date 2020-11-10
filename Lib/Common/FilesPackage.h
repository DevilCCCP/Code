#pragma once

#include <QJsonDocument>

#include <Lib/Include/Common.h>


class FilesPackage
{
  QJsonDocument mDocument;
  bool          mIsValid;
  QString       mError;

public:
  bool IsValid() const { return mIsValid; }
  const QString& ErrorString() const { return mError; }

public:
  bool PackDir(const QString& path);
  bool UnPackDir(const QString& path);

  bool Load(const QByteArray& data);
  bool Save(QByteArray& data);

  bool GetDirList(QStringList& dirList);
  bool GetFileList(QStringList& fileList);
  bool GetFileData(QByteArray& data);

public:
  explicit FilesPackage(const QString& path);
  explicit FilesPackage();
};
