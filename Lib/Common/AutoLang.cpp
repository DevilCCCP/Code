#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QDataStream>

#include "AutoLang.h"


bool AutoLang::LoadDir(const QString& path)
{
  QDir dir(path);
  QFile infoFile(dir.absoluteFilePath("info.txt"));
  if (!infoFile.open(QFile::ReadOnly)) {
    return false;
  }

  while (!infoFile.atEnd()) {
    QString line = QString::fromLatin1(infoFile.readLine());
    QStringList row = line.split('\t');
    if (row.size() == 2) {
      mLangList.append(row.at(0));
      mLangRate.append(row.at(1).toInt());
    }
  }

  QRegExp codeRegExp("&#(\\d{1,});");
  for (int i = 0; i < mLangList.size(); i++) {
    QFile langFile(dir.absoluteFilePath(mLangList.at(i) + ".txt"));
    if (!langFile.open(QFile::ReadOnly)) {
      return false;
    }

    while (!langFile.atEnd()) {
      QString line = QString::fromLatin1(langFile.readLine());
      QStringList row = line.split('\t');
      if (row.size() == 3) {
        int pos = codeRegExp.indexIn(row.at(2));
        if (pos >= 0) {
          QString charCode = codeRegExp.cap(1);
          ushort code = charCode.toShort();
          mAlpaMap[code].append(i);
        }
      }
    }
  }

  return true;
}

bool AutoLang::Save(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QFile::WriteOnly)) {
    return false;
  }

  QDataStream stream(&file);
  stream << mLangList;
  stream << mLangRate;
  stream << mAlpaMap;
  return stream.status() == QDataStream::Ok;
}

bool AutoLang::Load(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QFile::ReadOnly)) {
    return false;
  }

  QDataStream stream(&file);
  stream >> mLangList;
  stream >> mLangRate;
  stream >> mAlpaMap;
  return stream.status() == QDataStream::Ok;
}

int AutoLang::TestString(const QString& text)
{
  QVector<int> totalRate(mLangList.size(), (int)0);
  foreach (const QChar ch, text) {
    auto itr = mAlpaMap.find(ch.unicode());
    if (itr != mAlpaMap.end()) {
      const QVector<int>& values = itr.value();
      foreach (int value, values) {
        totalRate[value]++;
      }
    }
  }

  mLastLang = -1;
  int topRate = 0;
  int topLang = 0;
  for (int i = 0; i < totalRate.size(); i++) {
    int rate = totalRate.at(i);
    if (rate > topRate || ((rate == topRate) && mLangRate.at(i) > topLang)) {
      topRate = rate;
      topLang = mLangRate.at(i);
      mLastLang = i;
    }
  }
  return mLastLang;
}

QString AutoLang::LastLangName()
{
  return mLangList.value(mLastLang);
}


AutoLang::AutoLang()
  : mLastLang(0)
{
}

