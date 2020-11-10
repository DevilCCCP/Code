#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#include "FilesPackage.h"


bool FilesPackage::PackDir(const QString& path)
{
  mDocument = QJsonDocument();

  QJsonArray rootArray;
  QDir rootDir(path);
  QDirIterator iter(path, QStringList()
                    , QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot
                    , QDirIterator::Subdirectories);
  while (iter.hasNext()) {
    QString filePath = iter.next();
    QString relPath = rootDir.relativeFilePath(filePath);
    QFileInfo info(filePath);
    if (info.isFile()) {
      QFile file(filePath);
      if (!file.open(QFile::ReadOnly)) {
#ifdef LANG_EN
        mError = QString("Open file fail (path: '%1', error: '%2')").arg(filePath, file.errorString());
#else
        mError = QString("Открытие файла неудачно (путь: '%1', ошибка: '%2')").arg(filePath, file.errorString());
#endif
        return false;
      }
      QByteArray data = file.readAll();

      QJsonObject jsonObject;
      jsonObject.insert("Path", QJsonValue(relPath));
      jsonObject.insert("IsDir", QJsonValue(false));
      jsonObject.insert("Data", QJsonValue(QString::fromLatin1(data.toBase64())));
      rootArray.append(jsonObject);
    } else {
      QJsonObject jsonObject;
      jsonObject.insert("Path", QJsonValue(relPath));
      jsonObject.insert("IsDir", QJsonValue(true));
      rootArray.append(jsonObject);
    }
  }
  mDocument.setArray(rootArray);
  mIsValid = true;
  mError.clear();
  return true;
}

bool FilesPackage::UnPackDir(const QString& path)
{
  QDir rootDir(path);
  QJsonArray rootArray = mDocument.array();
  for (int i = 0; i < rootArray.size(); i++) {
    QJsonObject jsonObject = rootArray[i].toObject();
    if (jsonObject.isEmpty()) {
#ifdef LANG_EN
      mError = QString("Bad syntax");
#else
      mError = QString("Неверный формат данных");
#endif
      return false;
    }

    if (jsonObject.value("IsDir").toBool()) {
      QString path = jsonObject.value("Path").toString();
      QString absPath = rootDir.absoluteFilePath(path);
      if (!rootDir.mkpath(absPath)) {
#ifdef LANG_EN
        mError = QString("Make path '%1' fail").arg(absPath);
#else
        mError = QString("Невозможно создать путь '%1'").arg(absPath);
#endif
        return false;
      }
    }
  }
  for (int i = 0; i < rootArray.size(); i++) {
    QJsonObject jsonObject = rootArray[i].toObject();

    if (!jsonObject.value("IsDir").toBool()) {
      QString path = jsonObject.value("Path").toString();
      QString absPath = rootDir.absoluteFilePath(path);
      QDir dir(absPath);
      dir.cdUp();
      if (!dir.mkpath(dir.absolutePath())) {
#ifdef LANG_EN
        mError = QString("Make path '%1' fail").arg(dir.absolutePath());
#else
        mError = QString("Невозможно создать путь '%1'").arg(dir.absolutePath());
#endif
        return false;
      }
      QFile file(absPath);
      if (!file.open(QFile::WriteOnly)) {
#ifdef LANG_EN
        mError = QString("Create file '%1' fail").arg(absPath);
#else
        mError = QString("Невозможно создать файл '%1'").arg(absPath);
#endif
        return false;
      }
      QByteArray data = QByteArray::fromBase64(jsonObject.value("Data").toString().toLatin1());
      if (file.write(data) != data.size()) {
#ifdef LANG_EN
        mError = QString("Write file '%1' fail").arg(absPath);
#else
        mError = QString("Невозможно записать файл '%1'").arg(absPath);
#endif
        return false;
      }
    }
  }
  mError.clear();
  return true;
}

bool FilesPackage::Load(const QByteArray& data)
{
  QByteArray planeData = qUncompress(data);
  if (planeData.isEmpty()) {
    mIsValid = false;
#ifdef LANG_EN
      mError = QString("Bad compression");
#else
      mError = QString("Ошибка архива");
#endif
    return false;
  }
  QJsonParseError err;
  mDocument = QJsonDocument::fromJson(planeData, &err);
  if (err.error != QJsonParseError::NoError) {
    mIsValid = false;
    mError = err.errorString();
    return false;
  }

  mIsValid = true;
  mError.clear();
  return true;
}

bool FilesPackage::Save(QByteArray& data)
{
  QByteArray planeData = mDocument.toJson();
  data = qCompress(planeData);
  return true;
}

bool FilesPackage::GetDirList(QStringList& dirList)
{
  QJsonArray rootArray = mDocument.array();
  for (int i = 0; i < rootArray.size(); i++) {
    QJsonObject jsonObject = rootArray[i].toObject();
    if (jsonObject.isEmpty()) {
#ifdef LANG_EN
      mError = QString("Bad syntax");
#else
      mError = QString("Неверный формат данных");
#endif
      return false;
    }

    if (jsonObject.value("IsDir").toBool()) {
      QString path = jsonObject.value("Path").toString();
      dirList.append(path);
    }
  }
  mError.clear();
  return true;
}

bool FilesPackage::GetFileList(QStringList& fileList)
{
  QJsonArray rootArray = mDocument.array();
  for (int i = 0; i < rootArray.size(); i++) {
    QJsonObject jsonObject = rootArray[i].toObject();
    if (jsonObject.isEmpty()) {
#ifdef LANG_EN
      mError = QString("Bad syntax");
#else
      mError = QString("Неверный формат данных");
#endif
      return false;
    }

    if (!jsonObject.value("IsDir").toBool()) {
      QString path = jsonObject.value("Path").toString();
      fileList.append(path);
    }
  }
  mError.clear();
  return true;
}


FilesPackage::FilesPackage(const QString& path)
{
  mIsValid = PackDir(path);
}

FilesPackage::FilesPackage()
  : mIsValid(false)
{
}
