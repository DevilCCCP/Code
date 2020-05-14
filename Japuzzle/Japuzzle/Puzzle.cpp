#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QFileInfo>

#include "Puzzle.h"
#include "Core.h"
#include "Account.h"
#include "Decoration.h"
#include "Editing.h"
#include "GameState.h"


void Puzzle::SetName(const QString& _SourceName)
{
  mSourceName = _SourceName;
  mViewName   = QFileInfo(mSourceName).fileName();
}

QString Puzzle::StarsText()
{
  switch (mStars) {
  case 0: return "0 звёзд";
  case 1: return "1 звезда";
  case 2: return "2 звезды";
  case 3: return "3 звезды";
  case 4: return "4 звезды";
  case 5: return "5 звёзд";
  }
  return "X звёзд";
}

void Puzzle::New(int width, int height)
{
  mViewName = QString("Рисунок.ypp");
  mWidth  = width;
  mHeight = height;
  mTable.resize(mWidth * mHeight);
  mTable.fill(Cell());
  ClearUndo();
}

void Puzzle::Reset()
{
  for (int i = 0; i < mProp.size(); i++) {
    mProp[i].Used = false;
  }
  for (int i = 0; i < mWidth * mHeight; i++) {
    mTable[i].Clear();
  }
}

bool Puzzle::IsBlank()
{
  if (IsEmpty()) {
    return true;
  }
  for (int i = 0; i < mWidth * mHeight; i++) {
    if (mTable.at(i).Real()) {
      return false;
    }
  }
  return true;
}

void Puzzle::Clear()
{
  MakeUndo();

  for (int i = 0; i < mProp.size(); i++) {
    mProp[i].Used = false;
  }
  for (int i = 0; i < mWidth * mHeight; i++) {
    mTable[i].Clear();
  }
  for (int i = 0; i < mWidth; i++) {
    mDigitsMarkHorz[i].fill(0);
    mDigitsMarkVert[i].fill(0);
  }
}

void Puzzle::ClearPropMark()
{
  for (int i = 0; i < mWidth * mHeight; i++) {
    if (mTable[i].IsMarkProp()) {
      mTable[i].SetMark(0, 0);
    }
  }
}

void Puzzle::Copy(const Puzzle& puzzle)
{
  mWidth      = puzzle.mWidth;
  mHeight     = puzzle.mHeight;
  mTable      = puzzle.mTable;
  mDigitsHorz = puzzle.mDigitsHorz;
  mDigitsVert = puzzle.mDigitsVert;
}

void Puzzle::Apply(const Puzzle& puzzle)
{
  MakeUndo();
  mTable      = puzzle.mTable;
}

bool Puzzle::Load(const QString& filename, bool compact)
{
  SetName(filename);
  QFile file(filename);
  if (!file.open(QFile::ReadOnly)) {
    qCore->Warning(QString("Не удалось открыть файл '%1' (%2)").arg(filename, file.errorString()));
    return false;
  }

  QDataStream stream(&file);
  mStream = &stream;
  QTextStream txtStream(&file);
  mTxtStream = &txtStream;
  mStream->setByteOrder(QDataStream::LittleEndian);

  QByteArray header(7, (char)0);
  mStream->readRawData(header.data(), header.size());
  bool ok = false;
  if (header == "AL01YPP") {
    mDigits = false;
    ok = LoadYpp(compact);
  } else if (header == "AL02YPP") {
    mDigits = true;
    ok = LoadYpp(compact);
  } else {
    mTxtStream->seek(0);
    ok = LoadTxt();
  }
  if (ok) {
    ClearUndo();
    qGameState->SolveClear();
    qGameState->StateChange(GameState::eNoSolve);
    SolveTest();
    qCore->Info(QString("Файл '%1' успешно загружен").arg(filename));
  } else {
    qCore->Warning(QString("Файл '%1' повреждён").arg(filename));
  }
  return ok;
}

bool Puzzle::Save(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QFile::WriteOnly)) {
    qCore->Warning(QString("Неудалось открыть на запись файл '%1' (ошибка: '%2')").arg(filename, file.errorString()));
    return false;
  }

  QDataStream stream(&file);
  mStream = &stream;
  mStream->setByteOrder(QDataStream::LittleEndian);

  if (!SaveYpp()) {
    qCore->Warning(QString("Неудалось сохранить файл '%1' (ошибка: '%2')").arg(filename, file.errorString()));
    return false;
  }
  return true;
}

int Puzzle::Count() const
{
  int count = 0;
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      const Cell& cell = At(i, j);
      if (cell.HasMark()) {
        count++;
      }
    }
  }
  return count;
}

int Puzzle::Size() const
{
  return mWidth * mHeight;
}

void Puzzle::SetEditing(const EditingS& _Editing)
{
  mEditing = _Editing;
  if (mEditing) {
    mEditing->UndoChanged(HasUndo(), HasRedo());
  }
}

void Puzzle::SetCells(const QPoint& p1, const QPoint& p2, int mark, int level)
{
  MakeUndo();
  if (level > 0 && !mProp.at(level - 1).Used) {
    mProp[level - 1].Used  = true;
    mProp[level - 1].Pos   = p1;
    mProp[level - 1].Cell1 = At(p1);
    mProp[level - 1].Cell1.SetMark(mark, level);
  }
  bool allEqual = true;
  int x1 = qMin(p1.x(), p2.x());
  int x2 = qMax(p1.x(), p2.x());
  int y1 = qMin(p1.y(), p2.y());
  int y2 = qMax(p1.y(), p2.y());
  for (int j = y1; j <= y2; j++) {
    for (int i = x1; i <= x2; i++) {
      Cell* cell = &Value(i, j);
      if (cell->GetMark() != mark) {
        allEqual = false;
      }
      cell->SetMark(mark, level);
      for (int k = level; k < mProp.size(); k++) {
        if (mProp.at(k).Used && mProp.at(k).Pos.x() == i && mProp.at(k).Pos.y() == j) {
          mProp[k].Used = false;
        }
      }
    }
  }
  if (allEqual && mark != 0) {
    SetCells(p1, p2, 0, level);
  }
  SolveTest();
}

void Puzzle::ClearProp(int level)
{
  MakeUndo();
  int count = 0;
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      Cell* cell = &Value(i, j);
      if (cell->HasMark() && cell->MarkLevel() >= qMax(level, 1)) {
        cell->Clear();
        count++;
      }
    }
  }

  if (level > 0 && mProp[level-1].Used) {
    if (count > 1) {
      const QPoint& pos = mProp[level-1].Pos;
      if (IsValid(pos)) {
        Value(pos) = mProp[level-1].Cell1;
      }
    }
    mProp[level-1].Used = false;
  }
}

void Puzzle::ApplyProp(int level)
{
  MakeUndo();
  int count = 0;
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      Cell* cell = &Value(i, j);
      if (cell->HasMark() && cell->MarkLevel() >= qMax(level, 1)) {
        cell->SetMark(cell->GetMark(), qMax(level - 1, 0));
        count++;
      }
    }
  }

  if (level > 0 && mProp[level-1].Used) {
    mProp[level-1].Used = false;
  }
}

int Puzzle::GetAutoPropLevel()
{
  int level = 0;
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      const Cell& cell = At(i, j);
      level = qMax(level, cell.MarkLevel());
    }
  }
  return level;
}

bool Puzzle::SolveTest(bool isAi)
{
  bool realSolved = false;
  if (!CalcSolve(realSolved)) {
    return false;
  }

  if (qGameState->getState() > GameState::eBadSolve) {
    return realSolved;
  }

  if (isAi) {
    qGameState->StateChange(GameState::eAiSolve);
  } else if (!mDigits) {
    if (realSolved) {
      qGameState->StateChange(GameState::eManSolve);
    } else {
      qGameState->StateChange(GameState::eBadSolve);
    }
  } else {
    qGameState->StateChange(GameState::eUnknownSolve);
  }

  return realSolved;
}

bool Puzzle::CalcSolve(bool& realSolved)
{
  realSolved = true;
  for (int j = 0; j < mHeight; j++) {
    int currentI = 0;
    int count = -1;
    for (int i = 0; i < mWidth; i++) {
      if (At(i, j).IsMarkYes() != At(i, j).Real()) {
        realSolved = false;
      }
      if (At(i, j).IsMarkYes()) {
        if (count > 0) {
          count++;
        } else {
          count = 1;
        }
      } else {
        if (count > 0) {
          if (mDigitsHorz[currentI++][j] != count) {
            return false;
          }
          count = -1;
        }
      }
    }
    if (count > 0) {
      if (mDigitsHorz[currentI++][j] != count) {
        return false;
      }
    }
    if (mDigitsHorz[currentI][j] > 0) {
      return false;
    }
  }

  for (int i = 0; i < mWidth; i++) {
    int currentJ = 0;
    int count = -1;
    for (int j = 0; j < mHeight; j++) {
      if (At(i, j).IsMarkYes()) {
        if (count > 0) {
          count++;
        } else {
          count = 1;
        }
      } else {
        if (count > 0) {
          if (mDigitsVert[i][currentJ++] != count) {
            return false;
          }
          count = -1;
        }
      }
    }
    if (count > 0) {
      if (mDigitsVert[i][currentJ++] != count) {
        return false;
      }
    }
    if (mDigitsVert[i][currentJ] > 0) {
      return false;
    }
  }
  return true;
}

void Puzzle::SetDigit(Qt::Orientation type, const QPoint& p1, const QPoint& p2, int value)
{
  if (p1.x() == p2.x()) {
    int i = p1.x();
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    for (int j = y1; j <= y2; j++) {
      SetDigit(type, QPoint(i, j), value);
    }
  } else if (p1.y() == p2.y()) {
    int j = p1.y();
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    for (int i = x1; i <= x2; i++) {
      SetDigit(type, QPoint(i, j), value);
    }
  } else {
    SetDigit(type, p2, value);
  }
}

void Puzzle::SetDigit(Qt::Orientation type, const QPoint& p, int value)
{
  QVector<Line>* digitsMark = type == Qt::Horizontal? &mDigitsMarkHorz: &mDigitsMarkVert;
  (*digitsMark)[p.x()][p.y()] = value;
}

void Puzzle::ClearUndo()
{
  mUndoStack.clear();
  mCurrentUndo = nullptr;
  if (mEditing) {
    mEditing->UndoChanged(HasUndo(), HasRedo());
  }
  mOneUndoSize = sizeof(UndoInfo) + mWidth * mHeight * sizeof(Cell) + 3*sizeof(Propotion);
}

void Puzzle::MakeUndo()
{
  while (mUndoIndex < mUndoStack.size()) {
    mUndoStack.removeLast();
  }

  mUndoStack.append(UndoInfo());
  while (mUndoStack.size() * mOneUndoSize > qAccount->getUndoStackLimit() * 1024 * 1024) {
    mUndoStack.removeFirst();
  }
  mCurrentUndo = &mUndoStack.last();
  mUndoIndex = mUndoStack.size();

  mCurrentUndo->Cells     = mTable;
  mCurrentUndo->Prop      = mProp;

  mEditing->UndoChanged(HasUndo(), HasRedo());
}

bool Puzzle::HasUndo()
{
  return mUndoIndex > 0;
}

bool Puzzle::HasRedo()
{
  return mUndoIndex < mUndoStack.size() - 1;
}

bool Puzzle::DoUndo()
{
  if (!HasUndo()) {
    return false;
  }

  if (mUndoIndex >= mUndoStack.size()) {
    MakeUndo();
    mUndoIndex -= 2;
  } else {
    mUndoIndex--;
  }

  ApplyUndo();
  return true;
}

bool Puzzle::DoRedo()
{
  if (!HasRedo()) {
    return false;
  }

  mUndoIndex++;

  ApplyUndo();
  return true;
}

void Puzzle::Resize(const QRect& realRect)
{
  ClearUndo();

  int newWidth  = realRect.width();
  int newHeight = realRect.height();

  QVector<Cell> newTable;
  newTable.resize(newWidth * newHeight);
  for (int j = 0; j < newHeight; j++) {
    for (int i = 0; i < newWidth; i++) {
      newTable[i + j * newWidth] = mTable.value((i + realRect.left()) + (j + realRect.top()) * mWidth);
    }
  }

  mWidth  = newWidth;
  mHeight = newHeight;
  mTable          = newTable;
  mDigitsHorz     = mDigitsHorz.mid(realRect.left(), realRect.width());
  mDigitsMarkHorz = mDigitsMarkHorz.mid(realRect.left(), realRect.width());
  mDigitsVert     = mDigitsVert.mid(realRect.left(), realRect.width());
  mDigitsMarkVert = mDigitsMarkVert.mid(realRect.left(), realRect.width());
  mDigitsHorz.resize(mWidth);
  mDigitsMarkHorz.resize(mWidth);
  mDigitsVert.resize(mWidth);
  mDigitsMarkVert.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsHorz[i]     = mDigitsHorz[i].mid(realRect.top(), realRect.height());
    mDigitsMarkHorz[i] = mDigitsMarkHorz[i].mid(realRect.top(), realRect.height());
    mDigitsVert[i]     = mDigitsVert[i].mid(realRect.top(), realRect.height());
    mDigitsMarkVert[i] = mDigitsMarkVert[i].mid(realRect.top(), realRect.height());
    mDigitsHorz[i].resize(mHeight);
    mDigitsMarkHorz[i].resize(mHeight);
    mDigitsVert[i].resize(mHeight);
    mDigitsMarkVert[i].resize(mHeight);
  }
}

void Puzzle::ToByteArray(QByteArray& data)
{
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      char b = 0;
      mTable[i + j * mWidth].Save(b);
      data.append(b);
    }
  }
}

void Puzzle::FromByteArray(const QByteArray& data)
{
  if (data.size() != mWidth * mHeight) {
    return;
  }

  int l = 0;
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      char b = data.at(l);
      mTable[i + j * mWidth].Load(b);
      l++;
    }
  }
}

void Puzzle::ApplyUndo()
{
  mCurrentUndo = &mUndoStack[mUndoIndex];
  if (mCurrentUndo->Cells.size() != mWidth * mHeight) {
    return;
  }

  mTable = mCurrentUndo->Cells;
  mProp  = mCurrentUndo->Prop;

  mEditing->UndoChanged(HasUndo(), HasRedo());
}

bool Puzzle::LoadYpp(bool compact)
{
  *mStream >> mWidth;
  *mStream >> mHeight;
  if (!ValidateSize()) {
    return false;
  }
  mTable.resize(mWidth * mHeight);
  for (int i = 0; i < mWidth; i++) {
    for (int j = 0; j < mHeight; j++) {
      char ch;
      if (mStream->readRawData(&ch, 1) <= 0) {
        return false;
      }
      if (!mTable[i + j * mWidth].Load(ch)) {
        return false;
      }
    }
  }

  if (mDigits) {
    LoadDigits();
  } else {
    CalcDigits();
  }

  struct StorePropotion {
    int Q;
    int I;
    int J;

    StorePropotion(): Q(-1), I(-1), J(-1) { }
  };

  StorePropotion storeProp[3];
  *mStream >> storeProp[0].Q;
  *mStream >> storeProp[1].Q;
  *mStream >> storeProp[2].Q;
  *mStream >> storeProp[0].I;
  *mStream >> storeProp[0].J;
  *mStream >> storeProp[1].I;
  *mStream >> storeProp[1].J;
  *mStream >> storeProp[2].I;
  *mStream >> storeProp[2].J;

  for (int i = 0; i < mProp.size(); i++) {
    char ch = (char)(uchar)storeProp[i].Q;
    mProp[i].Cell1.Load(ch);
    mProp[i].Used     = storeProp[i].Q >= 0;
    mProp[i].Pos.rx() = storeProp[i].I;
    mProp[i].Pos.ry() = storeProp[i].J;
  }
  int fileVer;
  *mStream >> fileVer;
  do {
    mStars = 0;

    if (fileVer < 100) {
      break;
    }

    *mStream >> mStars;

    if (fileVer < 200) {
      break;
    }

    bool hasDigitMarks;
    *mStream >> hasDigitMarks;
    if (hasDigitMarks) {
      QVector<char> digitMarks(mWidth * mHeight, 0);

      mStream->readRawData(digitMarks.data(), mWidth * mHeight);
      for (int j = 0; j < mHeight; j++) {
        for (int i = 0; i < mWidth; i++) {
          mDigitsMarkHorz[i][j] = (digitMarks.at(j * mWidth + i))? 1: 0;
        }
      }

      mStream->readRawData(digitMarks.data(), mWidth * mHeight);
      for (int j = 0; j < mHeight; j++) {
        for (int i = 0; i < mWidth; i++) {
          mDigitsMarkVert[i][j] = (digitMarks.at(j * mWidth + i))? 1: 0;
        }
      }
    }

  } while (false);

  CalcMax();

  if (compact) {
    TryCompact();
  }
  return true;
}

bool Puzzle::LoadTxt()
{
  *mTxtStream >> mWidth;
  *mTxtStream >> mHeight;

  if (!ValidateSize()) {
    return false;
  }
  mTable.resize(mWidth * mHeight);

  mDigitsVert.resize(mWidth);
  mDigitsMarkVert.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsVert[i].resize(mHeight);
    mDigitsVert[i].fill(0);
    mDigitsMarkVert[i].resize(mHeight);
    mDigitsMarkVert[i].fill(0);
    for (int j = 0; j < mHeight; j++) {
      if (mTxtStream->atEnd()) {
        return false;
      }
      int count;
      *mTxtStream >> count;
      if (count > 0) {
        mDigitsVert[i][j] = count;
      } else {
        break;
      }
    }
  }

  mDigitsHorz.resize(mWidth);
  mDigitsMarkHorz.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsHorz[i].resize(mHeight);
    mDigitsHorz[i].fill(0);
    mDigitsMarkHorz[i].resize(mHeight);
    mDigitsMarkHorz[i].fill(0);
  }
  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      if (mTxtStream->atEnd()) {
        return false;
      }
      int count;
      *mTxtStream >> count;
      if (count > 0) {
        mDigitsHorz[i][j] = count;
      } else {
        break;
      }
    }
  }

  for (int i = 0; i < mProp.size(); i++) {
    mProp[i].Used     = false;
  }

  mDigits = true;
  CalcMax();
  return true;
}

bool Puzzle::SaveYpp()
{
  QByteArray header(mDigits? "AL02YPP": "AL01YPP");
  if (mStream->writeRawData(header.data(), header.size()) != header.size()) {
    return false;
  }

#define VALIDATE if (mStream->status() != QDataStream::Ok) return false
  *mStream << mWidth; VALIDATE;
  *mStream << mHeight; VALIDATE;
  if (mWidth > 400 || mHeight > 400) {
    return false;
  }
  for (int i = 0; i < mWidth; i++) {
    for (int j = 0; j < mHeight; j++) {
      char ch;
      if (!mTable[i + j * mWidth].Save(ch)) {
        return false;
      }
      if (mStream->writeRawData(&ch, 1) <= 0) {
        return false;
      }
    }
  }

  if (mDigits) {
    SaveDigits();
  }

  struct StorePropotion {
    int Q;
    int I;
    int J;

    StorePropotion(): Q(-1), I(-1), J(-1) { }
  };
  StorePropotion storeProp[3];
  for (int i = 0; i < mProp.size(); i++) {
    if (mProp[i].Used) {
      char ch = 0;
      mProp[i].Cell1.Save(ch);
      storeProp[i].Q = (int)(uchar)ch;
      storeProp[i].I = mProp[i].Pos.x();
      storeProp[i].J = mProp[i].Pos.y();
    } else {
      storeProp[i].Q = -1;
      storeProp[i].I = -1;
      storeProp[i].J = -1;
    }
  }

  *mStream << storeProp[0].Q;
  *mStream << storeProp[1].Q;
  *mStream << storeProp[2].Q;
  *mStream << storeProp[0].I;
  *mStream << storeProp[0].J;
  *mStream << storeProp[1].I;
  *mStream << storeProp[1].J;
  *mStream << storeProp[2].I;
  *mStream << storeProp[2].J;

  int fileVer = 200;
  *mStream << fileVer;
  *mStream << mStars;

  bool hasDigitMarks = (qAccount->getDigitStyle() == Account::eDigitManual);
  *mStream << hasDigitMarks;
  if (hasDigitMarks) {
    QVector<char> digitMarks(mWidth * mHeight, 0);
    for (int j = 0; j < mHeight; j++) {
      for (int i = 0; i < mWidth; i++) {
        digitMarks[j * mWidth + i] = (mDigitsMarkHorz.at(i).at(j) == 1)? 1: 0;
      }
    }
    mStream->writeRawData(digitMarks.data(), mWidth * mHeight);

    for (int j = 0; j < mHeight; j++) {
      for (int i = 0; i < mWidth; i++) {
        digitMarks[j * mWidth + i] = (mDigitsMarkVert.at(i).at(j) == 1)? 1: 0;
      }
    }
    mStream->writeRawData(digitMarks.data(), mWidth * mHeight);
  }
  return true;
}

bool Puzzle::ValidateSize()
{
  if (mWidth <= 0 || mWidth > 400 || mHeight <= 0 || mHeight > 400) {
    return false;
  }
  return true;
}

void Puzzle::CalcDigits()
{
  mDigitsHorz.resize(mWidth);
  mDigitsMarkHorz.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsHorz[i].resize(mHeight);
    mDigitsHorz[i].fill(0);
    mDigitsMarkHorz[i].resize(mHeight);
    mDigitsMarkHorz[i].fill(0);
  }
  for (int j = 0; j < mHeight; j++) {
    int currentI = 0;
    int count = -1;
    for (int i = 0; i < mWidth; i++) {
      if (At(i, j).Real()) {
        if (count > 0) {
          count++;
        } else {
          count = 1;
        }
      } else {
        if (count > 0) {
          mDigitsHorz[currentI++][j] = count;
          count = -1;
        }
      }
    }
    if (count > 0) {
      mDigitsHorz[currentI++][j] = count;
    }
  }

  mDigitsVert.resize(mWidth);
  mDigitsMarkVert.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsVert[i].resize(mHeight);
    mDigitsVert[i].fill(0);
    mDigitsMarkVert[i].resize(mHeight);
    mDigitsMarkVert[i].fill(0);
    int currentJ = 0;
    int count = -1;
    for (int j = 0; j < mHeight; j++) {
      if (At(i, j).Real()) {
        if (count > 0) {
          count++;
        } else {
          count = 1;
        }
      } else {
        if (count > 0) {
          mDigitsVert[i][currentJ++] = count;
          count = -1;
        }
      }
    }
    if (count > 0) {
      mDigitsVert[i][currentJ++] = count;
    }
  }
}

void Puzzle::LoadDigits()
{
  mDigitsHorz.resize(mWidth);
  mDigitsMarkHorz.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsHorz[i].resize(mHeight);
    mDigitsMarkHorz[i].resize(mHeight);
    mDigitsMarkHorz[i].fill(0);
    for (int j = 0; j < mHeight; j++) {
      uchar count;
      *mStream >> count;
      if (count > 0) {
        mDigitsHorz[i][j] = count;
      }
    }
  }

  mDigitsVert.resize(mWidth);
  mDigitsMarkVert.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    mDigitsVert[i].resize(mHeight);
    mDigitsMarkVert[i].resize(mHeight);
    mDigitsMarkVert[i].fill(0);
    for (int j = 0; j < mHeight; j++) {
      uchar count;
      *mStream >> count;
      if (count > 0) {
        mDigitsVert[i][j] = count;
      }
    }
  }
}

bool Puzzle::SaveDigits()
{
  for (int i = 0; i < mWidth; i++) {
    for (int j = 0; j < mHeight; j++) {
      uchar count = mDigitsHorz.at(i).value(j);
      *mStream << count; VALIDATE;
    }
  }

  mDigitsVert.resize(mWidth);
  for (int i = 0; i < mWidth; i++) {
    for (int j = 0; j < mHeight; j++) {
      uchar count = mDigitsVert.at(i).value(j);
      *mStream << count; VALIDATE;
    }
  }
  return true;
}

void Puzzle::CalcMax()
{
  mDigitsHorzMax = mDigitsVertMax = 0;

  for (int j = 0; j < mHeight; j++) {
    for (int i = 0; i < mWidth; i++) {
      if (!mDigitsHorz[i][j]) {
        mDigitsHorzMax = qMax(mDigitsHorzMax, i);
        break;
      }
    }
  }

  for (int i = 0; i < mWidth; i++) {
    for (int j = 0; j < mHeight; j++) {
      if (!mDigitsVert[i][j]) {
        mDigitsVertMax = qMax(mDigitsVertMax, j);
        break;
      }
    }
  }
}

void Puzzle::TryCompact()
{
  QRect fullRect(0, 0, mWidth, mHeight);
  QRect realRect(0, 0, mWidth, mHeight);

  for (int i = 0; i < mWidth; i++) {
    bool hasLine = false;
    for (int j = 0; j < mHeight; j++) {
      if (mDigitsVert[i][j] > 0) {
        hasLine = true;
        break;
      }
    }
    if (hasLine) {
      break;
    }
    realRect.setLeft(realRect.left() + 1);
  }

  for (int i = mWidth - 1; i >= 0; i--) {
    bool hasLine = false;
    for (int j = 0; j < mHeight; j++) {
      if (mDigitsVert[i][j] > 0) {
        hasLine = true;
        break;
      }
    }
    if (hasLine) {
      break;
    }
    realRect.setRight(realRect.right() - 1);
  }

  for (int j = 0; j < mHeight; j++) {
    bool hasLine = false;
    for (int i = 0; i < mWidth; i++) {
      if (mDigitsHorz[i][j] > 0) {
        hasLine = true;
        break;
      }
    }
    if (hasLine) {
      break;
    }
    realRect.setTop(realRect.top() + 1);
  }

  for (int j = mHeight - 1; j >= 0; j--) {
    bool hasLine = false;
    for (int i = 0; i < mWidth; i++) {
      if (mDigitsHorz[i][j] > 0) {
        hasLine = true;
        break;
      }
    }
    if (hasLine) {
      break;
    }
    realRect.setBottom(realRect.bottom() - 1);
  }

  if (realRect.left() >= realRect.right() || realRect.top() >= realRect.bottom()) {
    realRect = QRect(0, 0, 1, 1);
  }
  if (realRect != fullRect) {
    Resize(realRect);
  }
}


Puzzle::Puzzle()
  : mWidth(0), mHeight(0)
  , mDigitsHorzMax(0), mDigitsVertMax(0)
  , mProp(3), mStars(0), mDigits(false)
  , mCurrentUndo(nullptr), mUndoIndex(0)
{
}

#undef VALIDATE
