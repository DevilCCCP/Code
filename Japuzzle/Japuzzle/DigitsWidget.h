#pragma once

#include <Lib/Include/Common.h>
#include <Lib/CoreUi/QWidgetB.h>


class DigitsWidget: public QWidgetB
{
  Qt::Orientation   mOrientation;
  Qt::AlignmentFlag mAlign;

  PROPERTY_GET(int, Width)
  PROPERTY_GET(int, Height)

  int               mCellWidth;
  int               mCellHeight;

  bool              mCompactMode;
  int               mCompactCount;
  int               mCompactStart;
  int               mCompactFinish;
  int               mCompactTail;
  int               mModeSetDigit;
  QPoint            mLastDigitPoint;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  void Init(Qt::Orientation _Orientation, Qt::AlignmentFlag _Align);
  void Setup();
  void SetViewCut(int value);
  int ExpandToMinView();
  void UpdateHighlight();
  void UpdateDigits(const QPoint& p1, const QPoint& p2);

private:
  void DrawHorz(const QRect& rect);
  void DrawVert(const QRect& rect);

  void SetDigits(const QPoint& nextPos);
  bool TranslateDigit(const QPoint& pos, QPoint& p);

signals:
  void ChangedDigits(int type, const QPoint& p1, const QPoint& p2);

public:
  explicit DigitsWidget(QWidget* parent = 0);
};
