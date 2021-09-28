#pragma once

#include <QVector>
#include <QPoint>

#include <Lib/CoreUi/FormImageView.h>


class QSettings;
class FormImageLineRegion;

class FormImageLineView: public FormImageView
{
public:
  enum EMode {
    eSelect,
    eLine,
    eRectangle
  };

private:
  FormImageLineRegion* mFormImageLineRegion;
  EMode                mMode;
  QVector<QPointF>     mLinePoints;
  QVector<QPointF>     mRectPoints;

  Q_OBJECT

public:
  const QVector<QPoint> LinePoints() const;
  const QList<uchar> LineValues() const;
  const QVector<uchar> RectValues() const;

public:
  void SetMode(EMode _Mode);

  void SyncSettings(QSettings* settings);
  void RestoreSettings(QSettings* settings);

  void MoveLineX(int x);
  void MoveLineY(int y);

private:
  QString PointVarname(int i) const;

  void SyncLine(QSettings* settings) const;
  void SyncRect(QSettings* settings) const;
  void RestoreLine(QSettings* settings);
  void RestoreRect(QSettings* settings);

signals:
  void LineChanged();

public:
  FormImageLineView(QWidget* parent = 0);
};
