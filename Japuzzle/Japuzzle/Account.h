#pragma once

#include <QString>
#include <QList>

#include <Lib/Include/Common.h>

#include "AccountInfo.h"


DefineStructS(AccountInfo);
DefineStructS(QLockFile);

class Account
{
public:
  enum EPuzzleType {
    eWait,
    eDone,
    eTooHard,
    eTooEasy,
    eFail,
  };

  enum EDigitStyle {
    eDigitManual,
    eDigitAuto,
    eDigitSmart,
  };

  enum ECalcWindow {
    eCalcWindowNone,
    eCalcWindowSimple,
    eCalcWindowSmart,
  };

  struct PuzzleInfo {
    QString     Filename;
    int         DirIndex;
    EPuzzleType Type;
  };

private:
  PROPERTY_GET(QString,           Name)
  PROPERTY_GET(QString,           ViewName)
  PROPERTY_GET(QString,           Path)
  PROPERTY_GET(QLockFileS,        Lock)

  PROPERTY_GET(QStringList,       PuzzleDirList)
  PROPERTY_GET(QList<PuzzleInfo>, PuzzleInfoList)

  PROPERTY_GET(int,               StyleIndex)
  PROPERTY_GET(int,               PreviewSize)

  PROPERTY_GET(QString,           LastPuzzle)
  PROPERTY_GET(QString,           TempPuzzle)
  PROPERTY_GET(int,               CurrentPuzzleDir)
  PROPERTY_GET(QString,           CurrentPuzzleFilename)
  PROPERTY_GET(QString,           CurrentPuzzlePath)
  PROPERTY_GET(int,               WaitCount)
  PROPERTY_GET(int,               DoneCount)

  PROPERTY_GET(bool,              DigitHighlight)
  PROPERTY_GET(int,               CompactDigits)
  PROPERTY_GET(EDigitStyle,       DigitStyle)
  PROPERTY_GET(ECalcWindow,       CalcWindow)
  PROPERTY_GET(int,               AutoSavePeriod)
  PROPERTY_GET(int,               UndoStackLimit)
  PROPERTY_GET(bool,              AutoOpenPropEx)
  PROPERTY_GET(bool,              AutoCalcStars)

  PROPERTY_GET(bool,              ShowGameStateDialog)
  ;
public:
  bool Init(const QString& defaultPath);
  bool Test(const AccountInfo& info, const QString& path);
  bool Load(const AccountInfo& info, const QString& path);
  bool Save();
  bool PrepareTemp();
  void UpdatePuzzleList();

  bool HaveLastPuzzle();
  bool TakeNextPuzzle(EPuzzleType type = eDone);
  bool TakeCurrentPuzzle();
  bool SelectNextPuzzle();
  bool CloseCurrentPuzzle(EPuzzleType type);

  bool FindPuzzle(int dirIndex, const QString& fileName, int& index);
  bool PuzzleChangeStateTo(int index, EPuzzleType type);
  bool RestartPuzzles();

  QString PuzzleDir() const;

  static const char* TypeToString(EPuzzleType type);

private:
  bool GetCurrentPuzzleFilename();
  bool GenerateLastPuzzleFilename();
  bool GenerateTempPuzzleFilename();

public:
  Account();

  friend class DialogSettings;
  friend class DialogGameState;
  friend class DialogSolve;
};
