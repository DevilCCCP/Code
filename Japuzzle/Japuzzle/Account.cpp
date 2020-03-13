#include <QVector>
#include <QSet>
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QLockFile>
#include <QSettings>

#include "Account.h"
#include "Core.h"


bool Account::Init(const QString& defaultPath)
{
  mPath     = defaultPath;
  return true;
}

bool Account::Test(const AccountInfo& info, const QString& path)
{
  Q_UNUSED(info);

  QDir dir(path);
  mLock.reset(new QLockFile(dir.absoluteFilePath("User.lock")));
  mLock->setStaleLockTime(0);
  if (!mLock->tryLock(0)) {
    return false;
  }
  return true;
}

bool Account::Load(const AccountInfo& info, const QString& path)
{
  mName     = info.Name;
  mViewName = info.ViewName;
  mPath     = path;

  QDir dir(mPath);
  QString userFilename = dir.absoluteFilePath("User.ini");

  QSettings accountSettings(userFilename, QSettings::IniFormat);
  mPreviewSize    = accountSettings.value("PreviewSize", 1).toInt();

  mStyleIndex     = accountSettings.value("StyleIndex", 0).toInt();
  mLastPuzzle     = accountSettings.value("LastPuzzle", QString()).toString();
  mCurrentPuzzle  = accountSettings.value("CurrentPuzzle", -1).toInt();

  mDigitStyle     = (EDigitStyle)accountSettings.value("DigitStyle", eDigitAuto).toInt();
  mDigitHighlight = accountSettings.value("DigitHighlight", true).toBool();
  mCompactDigits  = accountSettings.value("CompactDigits", 4).toInt();
  mCalcWindow     = (ECalcWindow)accountSettings.value("CalcWindow", eCalcWindowSimple).toInt();
  mAutoSavePeriod = accountSettings.value("AutoSavePeriod", 30*1000).toInt();
  mUndoStackLimit = accountSettings.value("UndoStackLimit", 64).toInt();

  mShowGameStateDialog = accountSettings.value("ShowGameStateDialog", true).toBool();
  mPuzzleDirList.clear();

  accountSettings.beginGroup(QString("PuzzlePath"));
  for (int i = 0; ; i++) {
    QString puzzlePath = accountSettings.value(QString::number(i)).toString();
    if (puzzlePath.isEmpty()) {
      break;
    }
    mPuzzleDirList.append(puzzlePath);
  }
  if (mPuzzleDirList.isEmpty()) {
    mPuzzleDirList.append(qApp->applicationDirPath() + "/Puzzles");
  }
  accountSettings.endGroup();

  mWaitCount = 0;
  mDoneCount = 0;
  QVector<QSet<QString> > dirPuzzlesList;
  dirPuzzlesList.resize(mPuzzleDirList.size());
  accountSettings.beginGroup(QString("Puzzle"));
  for (int i = 0; ; i++) {
    accountSettings.beginGroup(QString::number(i));
    PuzzleInfo info;
    info.Filename = accountSettings.value(QString("file")).toString();
    info.DirIndex = accountSettings.value(QString("dir")).toInt();
    info.Type     = (EPuzzleType)accountSettings.value(QString("type")).toInt();
    switch (info.Type) {
    case eWait: mWaitCount++; break;
    case eDone: mDoneCount++; break;
    default: break;
    }
    accountSettings.endGroup();
    if (info.Filename.isEmpty()) {
      break;
    }
    if (info.DirIndex >= 0 && info.DirIndex < dirPuzzlesList.size()) {
      dirPuzzlesList[info.DirIndex].insert(info.Filename);
      mPuzzleInfoList.append(info);
    }
  }
  accountSettings.endGroup();

  for (int dirInd = 0; dirInd < dirPuzzlesList.size(); dirInd++) {
    const QString& dirPath = mPuzzleDirList.at(dirInd);
    QSet<QString>& dirPuzzles = dirPuzzlesList[dirInd];
    QDirIterator itr(dirPath);
    while (itr.hasNext()) {
      QFileInfo fileInfo(itr.next());
      if (!fileInfo.isFile()) {
        continue;
      }
      QString filename = fileInfo.fileName();
      if (!dirPuzzles.remove(filename)) {
        PuzzleInfo info;
        info.Filename = filename;
        info.DirIndex = dirInd;
        info.Type     = eWait;
        mWaitCount++;
        mPuzzleInfoList.append(info);
      }
    }
    foreach (const QString& filename, dirPuzzles) {
      for (int i = 0; i < mPuzzleInfoList.size(); i++) {
        const PuzzleInfo& info = mPuzzleInfoList.at(i);
        if (info.DirIndex == dirInd && info.Filename == filename) {
          mPuzzleInfoList.removeAt(i);
          break;
        }
      }
    }
  }
  GetCurrentPuzzleFilename();
  return true;
}

bool Account::Save()
{
  QDir dir(mPath);
  QSettings accountSettings(dir.absoluteFilePath("User.ini"), QSettings::IniFormat);
  accountSettings.setValue("PreviewSize",    mPreviewSize);

  accountSettings.setValue("StyleIndex",    mStyleIndex);
  accountSettings.setValue("LastPuzzle",    mLastPuzzle);
  accountSettings.setValue("CurrentPuzzle", mCurrentPuzzle);

  accountSettings.setValue("DigitStyle", (int)mDigitStyle);
  accountSettings.setValue("DigitHighlight", mDigitHighlight);
  accountSettings.setValue("CompactDigits", mCompactDigits);
  accountSettings.setValue("CalcWindow", (int)mCalcWindow);
  accountSettings.setValue("AutoSavePeriod", mAutoSavePeriod);
  accountSettings.setValue("UndoStackLimit", mUndoStackLimit);

  accountSettings.setValue("ShowGameStateDialog", mShowGameStateDialog);

  accountSettings.beginGroup(QString("PuzzlePath"));
  for (int i = 0; i < mPuzzleDirList.size(); i++) {
    accountSettings.setValue(QString::number(i), mPuzzleDirList.at(i));
  }
  accountSettings.endGroup();

  accountSettings.beginGroup(QString("Puzzle"));
  for (int i = 0; i < mPuzzleInfoList.size(); i++) {
    accountSettings.beginGroup(QString::number(i));
    const PuzzleInfo& info = mPuzzleInfoList.at(i);
    accountSettings.setValue(QString("file"), info.Filename);
    accountSettings.setValue(QString("dir"), info.DirIndex);
    accountSettings.setValue(QString("type"), info.Type);
    accountSettings.endGroup();
  }
  accountSettings.endGroup();

  accountSettings.sync();
  return true;
}

bool Account::PrepareTemp()
{
  if (!GenerateTempPuzzleFilename()) {
    return false;
  }

  return true;
}

bool Account::HaveLastPuzzle()
{
  return GetCurrentPuzzleFilename() && QFileInfo(mLastPuzzle).exists();
}

bool Account::TakeNextPuzzle(Account::EPuzzleType type)
{
  if (!CloseCurrentPuzzle(type)) {
    qCore->Warning("Сохранение текущего файла не удалось");
    return false;
  }

  if (mPuzzleInfoList.isEmpty()) {
    qCore->Warning("Файлы с картинками отсутствуют");
    return false;
  }

  if (!SelectNextPuzzle()) {
    qCore->Warning("Ошибка выбора следующего рисунка");
    return false;
  }
  if (!GetCurrentPuzzleFilename() || !GenerateLastPuzzleFilename()) {
    qCore->Warning("Внутренняя ошибка генерации имён файлов");
    return false;
  }
  return true;
}

bool Account::TakeCurrentPuzzle()
{
  if (!GetCurrentPuzzleFilename() || !GenerateLastPuzzleFilename()) {
    qCore->Warning("Внутренняя ошибка генерации имён файлов");
    return false;
  }
  return true;
}

bool Account::SelectNextPuzzle()
{
  ++mCurrentPuzzle;
  for (; mCurrentPuzzle < mPuzzleInfoList.size(); ++mCurrentPuzzle) {
    if (mPuzzleInfoList.at(mCurrentPuzzle).Type == eWait) {
      return true;
    }
  }

  for (mCurrentPuzzle = 0; mCurrentPuzzle < mPuzzleInfoList.size(); ++mCurrentPuzzle) {
    if (mPuzzleInfoList.at(mCurrentPuzzle).Type == eWait) {
      return true;
    }
  }

  for (mCurrentPuzzle = 0; mCurrentPuzzle < mPuzzleInfoList.size(); ++mCurrentPuzzle) {
    PuzzleInfo* info = &mPuzzleInfoList[mCurrentPuzzle];
    if (info->Type == eDone) {
      info->Type = eWait;
    }
  }

  for (mCurrentPuzzle = 0; mCurrentPuzzle < mPuzzleInfoList.size(); ++mCurrentPuzzle) {
    if (mPuzzleInfoList.at(mCurrentPuzzle).Type == eWait) {
      return true;
    }
  }
  return false;
}

bool Account::CloseCurrentPuzzle(Account::EPuzzleType type)
{
  if (PuzzleChangeStateTo(mCurrentPuzzle, type)) {
    return Save();
  }
  return true;
}

bool Account::PuzzleChangeStateTo(int index, Account::EPuzzleType type)
{
  if (index >= 0 && index < mPuzzleInfoList.size()) {
    switch (mPuzzleInfoList[mCurrentPuzzle].Type) {
    case eWait: mWaitCount--; break;
    case eDone: mDoneCount--; break;
    default: break;
    }
    switch (type) {
    case eWait: mWaitCount++; break;
    case eDone: mDoneCount++; break;
    default: break;
    }
    mPuzzleInfoList[index].Type = type;
    return true;
  }
  return false;
}

bool Account::RestartPuzzles()
{
  mCurrentPuzzle = -1;
  if (!SelectNextPuzzle()) {
    qCore->Warning("Ошибка выбора следующего рисунка");
    return false;
  }
  if (!GetCurrentPuzzleFilename() || !GenerateLastPuzzleFilename()) {
    qCore->Warning("Внутренняя ошибка генерации имён файлов");
    return false;
  }
  return true;
}

QString Account::PuzzleDir() const
{
  return mPuzzleDirList.value(0);
}

const char* Account::TypeToString(Account::EPuzzleType type)
{
  switch (type) {
  case eWait   : return "в очереди";
  case eDone   : return "собрана";
  case eTooHard: return "сложный";
  case eTooEasy: return "простой";
  case eFail   : return "проблемы";
  }
  return "<ошибка>";
}

bool Account::GetCurrentPuzzleFilename()
{
  mCurrentPuzzleFilename.clear();
  if (mCurrentPuzzle >= 0 && mCurrentPuzzle < mPuzzleInfoList.size()) {
    const PuzzleInfo& info = mPuzzleInfoList.at(mCurrentPuzzle);
    int dirIndex = info.DirIndex;
    if (dirIndex >= 0 && dirIndex < mPuzzleDirList.size()) {
      QDir dir(mPuzzleDirList.at(dirIndex));
      if (dir.exists(info.Filename)) {
        mCurrentPuzzleFilename = dir.filePath(info.Filename);
        return true;
      }
    }
  }
  return false;
}

bool Account::GenerateLastPuzzleFilename()
{
  QDir dir(mPath);
  if (!mLastPuzzle.isEmpty()) {
    QFile::remove(mLastPuzzle);
  }
  mLastPuzzle = dir.filePath("CurrentPuzzle.ypp");
  if (!QFile::exists(mLastPuzzle) || QFile::remove(mLastPuzzle)) {
    return true;
  }
  for (int i = 0; i < 100; i++) {
    mLastPuzzle = dir.filePath(QString("CurrentPuzzle_%1.ypp").arg(i, 3, 10, QChar('0')));
    if (!QFile::exists(mLastPuzzle) || QFile::remove(mLastPuzzle)) {
      return true;
    }
  }
  return false;
}

bool Account::GenerateTempPuzzleFilename()
{
  QDir dir(mPath);
  if (!mTempPuzzle.isEmpty()) {
    QFile::remove(mTempPuzzle);
  }
  mTempPuzzle = dir.filePath("OpenedPuzzle.ypp");
  if (!QFile::exists(mTempPuzzle) || QFile::remove(mTempPuzzle)) {
    return true;
  }
  for (int i = 0; i < 100; i++) {
    mTempPuzzle = dir.filePath(QString("OpenedPuzzle_%1.ypp").arg(i, 3, 10, QChar('0')));
    if (!QFile::exists(mTempPuzzle) || QFile::remove(mTempPuzzle)) {
      return true;
    }
  }
  return false;
}


Account::Account()
  : mStyleIndex(0)
  , mCurrentPuzzle(-1)
  , mDigitHighlight(true), mCompactDigits(4), mDigitStyle(eDigitAuto), mCalcWindow(eCalcWindowSimple), mAutoSavePeriod(30*1000), mUndoStackLimit(64)
  , mShowGameStateDialog(true)
{
}

