#pragma once

#include <QThread>
#include <QVector>
#include <QString>
#include <QDir>

#include "Common.h"


class RenameWorker: public QThread
{
  PROPERTY_GET_SET(QString, Path)

  PROPERTY_GET_SET(bool,    Rename)
  PROPERTY_GET_SET(QString, RenameRegExp)
  PROPERTY_GET_SET(QString, RenameValue)

  PROPERTY_GET_SET(bool,    Numbers)
  PROPERTY_GET_SET(int,     FirstNumber)
  PROPERTY_GET_SET(int,     SecondNumber)
  PROPERTY_GET_SET(int,     ResizeNumbers)
  PROPERTY_GET_SET(bool,    SwapNumbers)

  volatile bool             mStop;
  QVector<QString>          mFileList;
  int                       mProgress;
  QDir                      mBaseDir;
  QRegExp                   mRenRegExp;
  QRegExp                   mNumberRegExp;

  QString                   mCurrentFilePath;
  QString                   mNewFilePath;

  Q_OBJECT

private:
  /*override */void run() override;

private:
  void Prepare();
  void Do();
  void DoFile();
  void DoRename();
  void DoNumbers();
  void DoNumbersAssign(int number, int size, int& index);

public slots:
  void Start();
  void Halt();

signals:
  void Progress(int percent);

public:
  RenameWorker(QObject* parent = 0);
};
