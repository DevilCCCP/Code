#pragma once

#include <QWidget>
#include <QPoint>

#include <Lib/Include/Common.h>
#include <Lib/CoreUi/QWidgetB.h>

#include "Cell.h"


namespace Ui {
class FormCalcPopup;
}

class FormCalcPopup: public QWidgetB
{
  Ui::FormCalcPopup* ui;

  int                mCellWidth;
  int                mCellHeight;

  Qt::Orientation    mOrientation;
  Qt::Alignment      mAlign;
  QVector<Line>      mDigits;
  QVector<Line>      mDigitsColors;

  Q_OBJECT

protected:
  /*override */virtual void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

public:
  void Setup();

private:
  void DrawSimple();
  void DrawSmart();

  void CalcHorzYes(const QPoint& p1, const QPoint& p2);
  void CalcVertYes(const QPoint& p1, const QPoint& p2);
  void CalcHorzNo(const QPoint& p1, const QPoint& p2);
  void CalcVertNo(const QPoint& p1, const QPoint& p2);

public slots:
  void OnShow(const QPoint& pos, const QPoint& p1, const QPoint& p2, int mark, int flag);
  void OnHide();

public:
  explicit FormCalcPopup(QWidget* parent = 0);
  ~FormCalcPopup();
};
