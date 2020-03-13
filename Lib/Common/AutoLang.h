#pragma once

#include <QString>
#include <QVector>
#include <QMap>


class AutoLang
{
  QVector<QString>            mLangList;
  QVector<int>                mLangRate;
  QMap<ushort, QVector<int> > mAlpaMap;

  int                         mLastLang;

public:
  bool LoadDir(const QString& path);
  bool Save(const QString& filename);
  bool Load(const QString& filename);

  int TestString(const QString& text);
  QString LastLangName();

public:
  AutoLang();
};
