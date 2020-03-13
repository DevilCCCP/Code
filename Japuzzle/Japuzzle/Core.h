#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include <Lib/Include/Common.h>


DefineClassS(Core);
DefineClassS(Ai);
DefineClassS(CoreInfo);
DefineClassS(Puzzle);
DefineClassS(Account);
DefineClassS(Style);
DefineStructS(AccountInfo);
DefineStructS(StyleInfo);
typedef QVector<AccountInfoS> AccountsInfo;
typedef QVector<StyleInfoS> StylesInfo;

class Core: public QObject
{
  static Core*               mSelf;
  AiS                        mAi;
  CoreInfo*                  mCoreInfo;

  PROPERTY_GET(QString,      ProgramName)
  PROPERTY_GET(QString,      FileName)
  PROPERTY_GET(QString,      FileNameDgt)

  PROPERTY_GET(PuzzleS,      Puzzle)
  PROPERTY_GET(AccountS,     Account)
  PROPERTY_GET(StyleS,       Style)

  PROPERTY_GET(AccountsInfo, AccountsInfo)
  PROPERTY_GET(StylesInfo,   StylesInfo)

  PROPERTY_GET(QString,      Version)
  PROPERTY_GET(bool,         TempFile)

  Q_OBJECT
  ;
public:
  static Core* Instance() { return mSelf; }
  void SetInformer(CoreInfo* _CoreInfo);

public:
  void Info(const QString& text);
  void Warning(const QString& text);
  void Error(const QString& text);

  bool InitAccounts();
  bool InitStyles();
  bool InitVersion();

  bool LoadPuzzle(const QString& filename);
  bool LoadAutoPuzzle();
  bool LoadNextPuzzle(int type = -1);
  bool TakeFirstPuzzle();
  bool TakeCurrentPuzzle();
  bool CreateAccount(const AccountInfoS& info);
  bool LoadAccount(const AccountInfoS& info);
  bool TestAccount(const AccountInfoS& info);
  bool LoadDefaultAccount();
  bool ApplyAccount(bool isDefault);
  bool CloseAccount();
  bool RemoveAccount(int index);
  bool LoadStyle(int index);
  void AutoSavePuzzle();

private:
  QString AccoutDir();
  bool SaveAccounts();

  void SetPuzzle(PuzzleS& puzzle);

signals:
  void PuzzleChanged();

public:
  Core();
};

#define qCore (Core::Instance())
#define qPuzzle (Core::Instance()->getPuzzle())
#define qAccount (Core::Instance()->getAccount())
#define qStyle (Core::Instance()->getStyle())
