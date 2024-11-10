#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QFile>
#include <QStringList>

#include "RenameWorker.h"


void RenameWorker::run()
{
  mProgress = 0;
  Prepare();
  Do();
}

void RenameWorker::Prepare()
{
  mBaseDir.setPath(mPath);
  QDirIterator itr(mPath, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
  while (itr.hasNext()) {
    if (mStop) {
      return;
    }
    mFileList.append(itr.next());
  }

  mProgress = 1;
  emit Progress(mProgress);

  if (mRename) {
    mRenRegExp.setPattern(mRenameRegExp);
  }
  if (mNumbers) {
    mNumberRegExp.setPattern("\\d+");
  }
}

void RenameWorker::Do()
{
  for (mCurrentFileIndex = 0; mCurrentFileIndex < mFileList.size(); mCurrentFileIndex++) {
    if (mStop) {
      return;
    }
    int newProgress = 1 + 99 * mCurrentFileIndex / mFileList.size();
    if (newProgress > mProgress) {
      mProgress = newProgress;
      emit Progress(mProgress);
    }

    mCurrentFilePath = mFileList.at(mCurrentFileIndex);
    mNewFilePath = mCurrentFilePath;
    DoFile();
  }
  emit Progress(100);
}

void RenameWorker::DoFile()
{
  if (mNumbers) {
    DoNumbers();
  }
  if (mRename) {
    DoRename();
  }
  if (mImageResize) {
    DoImageResize();
  }
  mBaseDir.rename(mCurrentFilePath, mNewFilePath);
}

void RenameWorker::DoRename()
{
  QString relativePath = mBaseDir.relativeFilePath(mNewFilePath);
  relativePath.replace(mRenRegExp, mRenameValue);
  QString newPath = mBaseDir.absoluteFilePath(relativePath);
  mBaseDir.rename(mCurrentFilePath, newPath);
}

void RenameWorker::DoNumbers()
{
  QFileInfo info(mCurrentFilePath);
  QString fileName = info.completeBaseName();
  QString fileExt  = info.suffix();
  if (info.fileName().size() > fileName.size() + fileExt.size()) {
    fileName.append('.');
  }
  QStringList parts;
  QList<int> numberIndex;
  for (int i = 0; i < fileName.size(); ) {
    int iNumber = mNumberRegExp.indexIn(fileName, i);
    if (iNumber < 0) {
      parts.append(fileName.mid(i));
      break;
    }
    parts.append(fileName.mid(i, iNumber - i));
    int len = mNumberRegExp.matchedLength();
    numberIndex.append(parts.size());
    parts.append(fileName.mid(iNumber, len));
    i = iNumber + len;
  }
  int firstIndex = -2;
  DoNumbersAssign(mFirstNumber, numberIndex.size(), firstIndex);
  int secondIndex = -2;
  if (mSecondNumber) {
    DoNumbersAssign(mSecondNumber, numberIndex.size(), secondIndex);
  }
  if (firstIndex < -1 && secondIndex < 0) {
    return;
  }

  if (mResizeNumbers > 0 && firstIndex >= 0) {
    int index = numberIndex[firstIndex];
    if (parts[index].size() < mResizeNumbers) {
      QString zeroString(mResizeNumbers - parts[index].size(), QChar('0'));
      parts[index].prepend(zeroString);
    }
  }
  if (mResizeNumbers > 0 && secondIndex >= 0) {
    int index = numberIndex[secondIndex];
    if (parts[index].size() < mResizeNumbers) {
      QString zeroString(mResizeNumbers - parts[index].size(), QChar('0'));
      parts[index].prepend(zeroString);
    }
  }

  if (mSwapNumbers && firstIndex >= 0 && secondIndex >= 0 && firstIndex != secondIndex) {
    qSwap(parts[numberIndex[firstIndex]], parts[numberIndex[secondIndex]]);
  }
  QString newFileName = parts.join("") + fileExt;
  if (firstIndex == -1) {
    int number = mResizeNumbers > 0? mResizeNumbers: 6;
    QString name = QString("%1").arg(mCurrentFileIndex + 1, number, 10, QChar('0'));
    newFileName = name + "." + fileExt;
  }
  QDir dir = info.dir();
  mNewFilePath = dir.absoluteFilePath(newFileName);
}

void RenameWorker::DoNumbersAssign(int number, int size, int& index)
{
  if (number == 0) {
    index = -1;
  } else if (number > 0) {
    if (number <= size) {
      index = number - 1;
    }
  } else {
    if (number <= size) {
      index = size + number;
    }
  }
}

void RenameWorker::DoImageResize()
{
  const QString kProg("/usr/bin/identify");
  QProcess procTest;
  procTest.setProgram(kProg);
  QStringList args = { "-format", "%m %w %h\\n", mCurrentFilePath };
  procTest.setArguments(args);
  procTest.start();
  if (!procTest.waitForFinished()) {
    procTest.kill();
    return;
  }
  QString out = QString::fromUtf8(procTest.readAllStandardOutput());
  QStringList whList = out.split(QChar(' '));
  QString type = whList.value(0);
  int w = whList.value(1).toInt();
  int h = whList.value(2).toInt();

  bool reType = type != "JPEG";
  bool reSize = w > mImageWidth || h > mImageHeight;
  if (!reType && !reSize) {
    return;
  }

  QString outFilePath = mCurrentFilePath;
  if (reType) {
    int ind = mCurrentFilePath.lastIndexOf(".");
    if (ind > 0) {
      outFilePath = mCurrentFilePath.mid(0, ind) + ".jpg";
    } else {
      reType = false;
    }
  }

  const QString kProg2("/usr/bin/convert");
  QProcess procResize;
  procResize.setProgram(kProg2);
  QStringList args2;
  if (reSize) {
    args2.append("-resize");
    args2.append(QString("%1x%2").arg(mImageWidth).arg(mImageHeight));
  }
  args2.append("-quality");
  args2.append(QString::number(mImageQuality));
  args2.append(mCurrentFilePath);
  args2.append(outFilePath);

  procResize.setArguments(args2);
  procResize.start();
  procResize.waitForFinished();
//  QByteArray data = procResize.readAllStandardOutput();
//  if (data.isEmpty()) {
//    return;
//  }

//  QFile file(mCurrentFilePath);
//  if (file.size() < data.size()) {
//    return;
//  }

//  if (file.open(QFile::WriteOnly)) {
//    file.write(data);
//    file.close();
//  }
  if (procResize.exitStatus() == QProcess::NormalExit && procResize.exitCode() == 0 && reType) {
    QFile::remove(mCurrentFilePath);
  }
}

void RenameWorker::Start()
{
  mStop = false;

  emit Progress(0);
  start();
}

void RenameWorker::Halt()
{
  mStop = true;
}


RenameWorker::RenameWorker(QObject* parent)
  : QThread(parent)
{
}
