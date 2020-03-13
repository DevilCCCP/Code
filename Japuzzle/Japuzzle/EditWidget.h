#pragma once

#include <QWidget>

#include <Lib/CoreUi/QWidgetB.h>
#include <Lib/Include/Common.h>


DefineClassS(Puzzle);
DefineClassS(Decoration);

class EditWidget: public QWidgetB
{
  PuzzleS           mPuzzle;
  DecorationS       mDecoration;
  PROPERTY_GET(int, Width)
  PROPERTY_GET(int, Height)

  int               mCellWidth;
  int               mCellHeight;
  QPoint            mCurrentPos;
  bool              mCurrentSet;
  int               mBrickSize;
  bool              mHasChanges;

  Q_OBJECT

public:
  void SetBrickSize(int value);
  bool HasChanges() { return mHasChanges; }
  void ClearChanges() { mHasChanges = false; }
  void SetChanges() { mHasChanges = true; }

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void Setup(const PuzzleS& _Puzzle, const DecorationS& _Decoration);
  void UpdateSize();

private:
  void UpdatePuzzle();
  bool TranslateCell(int x, int y);
  bool TranslatePointToCellX(int x, int& i);
  bool TranslatePointToCellY(int y, int& j);

  void SetCells(const QPoint& p, bool set);
  void UpdateCells(const QPoint& p1, const QPoint& p2);

signals:
  void UpdatePreview();
  void UpdateDigits(const QPoint& p1, const QPoint& p2);
  void ShowCalcWindow(const QPoint& pos, const QPoint& p1, const QPoint& p2, int mark, int flag);
  void MoveCalcWindow(const QPoint& pos);
  void HideCalcWindow();

public:
  explicit EditWidget(QWidget* parent = 0);
};
