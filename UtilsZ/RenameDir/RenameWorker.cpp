#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>

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
  for (int i = 0; i < mFileList.size(); i++) {
    if (mStop) {
      return;
    }
    int newProgress = 1 + 99 * i / mFileList.size();
    if (newProgress > mProgress) {
      mProgress = newProgress;
      emit Progress(mProgress);
    }

    mCurrentFilePath = mFileList.at(i);
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
  int firstIndex = -1;
  DoNumbersAssign(mFirstNumber, numberIndex.size(), firstIndex);
  int secondIndex = -1;
  if (mSecondNumber) {
    DoNumbersAssign(mSecondNumber, numberIndex.size(), secondIndex);
  }
  if (firstIndex < 0 && secondIndex < 0) {
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
  QDir dir = info.dir();
  mNewFilePath = dir.absoluteFilePath(newFileName);
}

void RenameWorker::DoNumbersAssign(int number, int size, int& index)
{
  if (number == 0) {
    if (size == 1) {
      index = 0;
    }
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
  QString convert = QString("convert %1 -resize %2x%3 -quality %4 > %1")
      .arg(mCurrentFilePath).arg(mImageWidth).arg(mImageHeight).arg(mImageQuality).arg(mCurrentFilePath);
  QProcess::execute(convert);
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
