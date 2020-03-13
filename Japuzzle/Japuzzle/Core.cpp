#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>

#include <Lib/CoreUi/Version.h>

#include "Core.h"
#include "Ai.h"
#include "CoreInfo.h"
#include "Puzzle.h"
#include "Account.h"
#include "Style.h"
#include "GameState.h"


Core* Core::mSelf = nullptr;

void Core::SetInformer(CoreInfo* _CoreInfo)
{
  mCoreInfo = _CoreInfo;
}

void Core::Info(const QString& text)
{
  if (mCoreInfo) {
    mCoreInfo->Info(text);
  }
}

void Core::Warning(const QString& text)
{
  if (mCoreInfo) {
    mCoreInfo->Warning(text);
  }
}

void Core::Error(const QString& text)
{
  if (mCoreInfo) {
    mCoreInfo->Error(text);
  }
}

bool Core::InitAccounts()
{
  QDir dir(AccoutDir());

  QSettings accountSettings(dir.absoluteFilePath("Account.ini"), QSettings::IniFormat);
  accountSettings.setIniCodec("UTF-8");
  for (int i = 0; ; i++) {
    QString name = accountSettings.value(QString("Account.%1").arg(i)).toString();
    if (name.isEmpty()) {
      break;
    }

    accountSettings.beginGroup(name);
    AccountInfoS info(new AccountInfo());
    info->Name        = name;
    info->ViewName    = accountSettings.value("Name").toString();
    info->Existed     = true;
    accountSettings.endGroup();
    mAccountsInfo.append(info);
  }
  return true;
}

bool Core::InitStyles()
{
  QDir dir(qApp->applicationDirPath());
  dir.cd("Styles");
  QSettings styleSettings(dir.absoluteFilePath("Styles.ini"), QSettings::IniFormat);
  styleSettings.setIniCodec("UTF-8");
  for (int i = 0; ; i++) {
    QString name = styleSettings.value(QString("Style.%1").arg(i)).toString();
    if (name.isEmpty()) {
      break;
    }

    styleSettings.beginGroup(name);
    StyleInfoS info(new StyleInfo());
    info->Name           = name;
    info->ViewName       = styleSettings.value("Name").toString();
    info->BackColor      = styleSettings.value("BackColor").toString().toUInt(0, 16);
    info->LineColor      = styleSettings.value("LineColor").toString().toUInt(0, 16);
    info->HighlightColor = styleSettings.value("HighlightColor").toString().toUInt(0, 16);
    info->DigitColor1    = styleSettings.value("DigitColor1").toString().toUInt(0, 16);
    info->DigitColor2    = styleSettings.value("DigitColor2").toString().toUInt(0, 16);
    info->DigitColor3    = styleSettings.value("DigitColor3").toString().toUInt(0, 16);
    info->PreviewColorY  = styleSettings.value("PreviewColorY").toString().toUInt(0, 16);
    info->PreviewColorN  = styleSettings.value("PreviewColorN").toString().toUInt(0, 16);
    info->PreviewColorY1 = styleSettings.value("PreviewColorY1").toString().toUInt(0, 16);
    info->PreviewColorN1 = styleSettings.value("PreviewColorN1").toString().toUInt(0, 16);
    info->PreviewLine    = styleSettings.value("PreviewLine").toString().toUInt(0, 16);
    for (int i = 0; i < 4; i++) {
      info->CursorYesSpot.append(styleSettings.value(QString("SpotY%1").arg(i, 2, 2, QChar('0'))).toPoint());
      info->CursorNoSpot.append(styleSettings.value(QString("SpotN%1").arg(i, 2, 2, QChar('0'))).toPoint());
    }
    info->CursorESpot = styleSettings.value(QString("SpotE")).toPoint();
    styleSettings.endGroup();
    mStylesInfo.append(info);
  }
  return true;
}

bool Core::InitVersion()
{
  Version ver;
  if (!ver.LoadFromThis()) {
    mVersion = QString("<недоступно>");
    return false;
  }

  mVersion = ver.ToString();
  return true;
}

bool Core::LoadPuzzle(const QString& filename)
{
  PuzzleS puzzle(new Puzzle());
  if (!puzzle->Load(filename)) {
    return false;
  }

  mTempFile = true;
  SetPuzzle(puzzle);
  qAi->CalcAllDigits(mPuzzle.data());
  return mAccount->PrepareTemp() && mPuzzle->Save(mAccount->getTempPuzzle());
}

bool Core::LoadAutoPuzzle()
{
  if (!mAccount->HaveLastPuzzle()) {
    return LoadNextPuzzle();
  }

  PuzzleS puzzle(new Puzzle());
  if (!puzzle->Load(mAccount->getLastPuzzle())) {
    if (mAccount->TakeCurrentPuzzle()) {
      if (!TakeCurrentPuzzle()) {
        return false;
      }
      qCore->Info(QString("Файл загружен без сохранения правок"));
    }
    return false;
  }

  mTempFile = false;
  SetPuzzle(puzzle);
  mPuzzle->SetName(mAccount->getCurrentPuzzleFilename());

  qAi->CalcAllDigits(mPuzzle.data());

  return true;
}

bool Core::LoadNextPuzzle(int type)
{
  qGameState->StateChange(GameState::eNoSolve);
  bool ok = (type >= 0)
      ? mAccount->TakeNextPuzzle((Account::EPuzzleType)type)
      : mAccount->TakeNextPuzzle();
  if (!ok) {
    return false;
  }

  return TakeCurrentPuzzle();
}

bool Core::TakeFirstPuzzle()
{
  mAccount->RestartPuzzles();
  return TakeCurrentPuzzle();
}

bool Core::TakeCurrentPuzzle()
{
  for (int i = 0; i < 10; i++) {
    PuzzleS puzzle(new Puzzle());
    if (puzzle->Load(mAccount->getCurrentPuzzleFilename())) {
      mTempFile = false;
      SetPuzzle(puzzle);
      mPuzzle->Clear();
      return mPuzzle->Save(mAccount->getLastPuzzle()) && mAccount->Save();
    }

    if (!mAccount->TakeNextPuzzle(Account::eFail)) {
      return false;
    }
  }
  return false;
}

bool Core::CreateAccount(const AccountInfoS& info)
{
  if (!CloseAccount()) {
    return false;
  }
  QDir dir(AccoutDir());
  if (dir.exists(info->Name) || !dir.mkpath(dir.filePath(info->Name)) || !dir.cd(info->Name)) {
    return false;
  }
  AccountS account(new Account());
  if (!account->Load(*info, dir.absolutePath()) || !account->Save()) {
    return false;
  }
  info->Existed = true;
  mAccountsInfo.append(info);
  if (!SaveAccounts()) {
    return false;
  }
  mAccount = account;
  Info(QString("Создан пользователь: ") + mAccount->getViewName());
  return ApplyAccount(false);
}

bool Core::LoadAccount(const AccountInfoS& info)
{
  if (!CloseAccount()) {
    return false;
  }
  QDir dir(AccoutDir());
  if (!dir.cd(info->Name)) {
    return CreateAccount(info);
  }
  AccountS account(new Account());
  if (!account->Test(*info, dir.absolutePath())) {
    Warning("Невозможно загрузить пользователя, вход уже выполнен.");
    return false;
  }
  if (!account->Load(*info, dir.absolutePath())) {
    Warning("Невозможно загрузить пользователя.");
    return false;
  }
  mAccount = account;
  Info(QString("Выбран пользователь: ") + mAccount->getViewName());
  return ApplyAccount(false);
}

bool Core::TestAccount(const AccountInfoS& info)
{
  QDir dir(AccoutDir());
  if (!dir.cd(info->Name)) {
    return false;
  }
  AccountS account(new Account());
  return account->Test(*info, dir.absolutePath());
}

bool Core::LoadDefaultAccount()
{
  if (!CloseAccount()) {
    return false;
  }

  AccountS account(new Account());
  if (!account->Init(AccoutDir())) {
    return false;
  }
  mAccount = account;
  Info(QString("Выбран режим без пользователя"));

  return ApplyAccount(true);
}

bool Core::ApplyAccount(bool isDefault)
{
  if (!LoadStyle(mAccount->getStyleIndex())) {
    Warning(QString("Не удалось загрузить стиль, вероятно программа не была корректно установлена."));
    return false;
  }

  return isDefault? true: LoadAutoPuzzle();
}

bool Core::CloseAccount()
{
  if (!mAccount) {
    return true;
  }
  if (mPuzzle) {
    if (!mPuzzle->Save(mAccount->getLastPuzzle())) {
      return false;
    }
  }

  mAccount.clear();
  return true;
}

bool Core::RemoveAccount(int index)
{
  if (index < 0 || index >= mAccountsInfo.size()) {
    return false;
  }
  const AccountInfoS& info = mAccountsInfo.at(index);
  QDir dir(AccoutDir());
  dir.cd(info->Name);
  if (!dir.removeRecursively()) {
    return false;
  }
  mAccountsInfo.remove(index);
  SaveAccounts();
  return true;
}

bool Core::LoadStyle(int index)
{
  StyleInfoS info = mStylesInfo.value(index);

  if (!info) {
    return false;
  }
  StyleS style(new Style());
  if (!style->Load(*info)) {
    return false;
  }
  mStyle = style;
  return true;
}

void Core::AutoSavePuzzle()
{
  if (!mTempFile) {
    if (mPuzzle && mPuzzle->Save(mAccount->getLastPuzzle())) {
      Info(QString("Автоматическое сохранение успешно"));
    }
  } else {
    if (mPuzzle && mPuzzle->Save(mAccount->getTempPuzzle())) {
      Info(QString("Автоматическое сохранение успешно"));
    }
  }
}

QString Core::AccoutDir()
{
  return QStandardPaths::writableLocation(
      #if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        QStandardPaths::AppDataLocation
      #else
        QStandardPaths::DataLocation
      #endif
        );
}

bool Core::SaveAccounts()
{
  QDir dir(AccoutDir());

  QSettings accountSettings(dir.absoluteFilePath("Account.ini"), QSettings::IniFormat);
  accountSettings.setIniCodec("UTF-8");
  for (int i = 0; ; i++) {
    QString key = QString("Account.%1").arg(i);
    if (i < mAccountsInfo.size()) {
      const AccountInfoS& info = mAccountsInfo.at(i);
      accountSettings.setValue(key, info->Name);

      accountSettings.beginGroup(info->Name);
      accountSettings.setValue("Name", info->ViewName);
      accountSettings.endGroup();
    } else if (accountSettings.contains(key)) {
      QString name = accountSettings.value(key).toString();
      accountSettings.remove(key);

      accountSettings.beginGroup(name);
      accountSettings.remove("Name");
      accountSettings.endGroup();
    } else {
      break;
    }
  }
  accountSettings.sync();
  return true;
}

void Core::SetPuzzle(PuzzleS& puzzle)
{
  mPuzzle = puzzle;

  emit PuzzleChanged();
}


Core::Core()
  : mAi(new Ai()), mCoreInfo(nullptr)
  , mProgramName("Японский рисунок"), mFileName("Картинка для игры \"Японский рисунок\""), mFileNameDgt("Цифры для игры \"Японский рисунок\"")
  , mTempFile(false)
{
  if (!mSelf) {
    mSelf = this;
  }

  InitAccounts();
  InitStyles();
  InitVersion();
}
