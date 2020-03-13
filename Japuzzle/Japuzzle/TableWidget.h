#pragma once

#include <QWidget>

#include <Lib/CoreUi/QWidgetB.h>
#include <Lib/Include/Common.h>


class TableWidget: public QWidgetB
{
  PROPERTY_GET(int, Width)
  PROPERTY_GET(int, Height)

  int               mCellWidth;
  int               mCellHeight;

  QPoint            mSpotPos;
  QPoint            mCurrentPos;
  int               mSpotMark;
  bool              mShowCalcWindow;
  QPoint            mShowCalcPos;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

  /*override */virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  /*override */virtual void leaveEvent(QEvent* event) Q_DECL_OVERRIDE;

public:
  void Setup();

private:
  bool TranslateCell(int x, int y);
  void UpdateCells(const QPoint& p1, const QPoint& p2);

  bool HasSpot() const { return mSpotMark != 0; }

signals:
  void UpdatePreview();
  void UpdateDigits(const QPoint& p1, const QPoint& p2);
  void ShowCalcWindow(const QPoint& pos, const QPoint& p1, const QPoint& p2, int mark, int flag);
  void MoveCalcWindow(const QPoint& pos);
  void HideCalcWindow();

public:
  explicit TableWidget(QWidget* parent = 0);
};
