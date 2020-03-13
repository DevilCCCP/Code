#pragma once

#include <QLabel>
#include <QTimer>

#include <Lib/Include/Common.h>


DefineClassS(Db);
DefineClassS(DbSettings);

enum EPointsType {
  eEmpty,
  eLine,
  eLineWithOrder,
  eLineZone,
  eAreaGood,
  eAreaIgnore,
};

class ImageWithPoints: public QLabel
{
  EPointsType    mPointsType;
  DbSettingsS    mPointSettings;
  int            mObjectId;
  QImage         mSourceImg;

  QList<QPointF> mPoints;
  QPointF        mPointEq;
  int            mSelIndex;
  QTimer         mDrawTimer;

  Q_OBJECT

protected:
  virtual void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

  virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

public:
  bool SetPointsType(const Db& _Db, int objectId, EPointsType _PointsType);
  void SetImage(const QImage& image);
  bool LoadPoints(bool redraw);
  void SavePoints();

private:
  bool IsTypeArea() { return mPointsType == eAreaGood || mPointsType == eAreaIgnore; }
  bool IsTypeLine() { return mPointsType == eLine || mPointsType == eLineWithOrder; }
  bool IsTypeLineZone() { return mPointsType == eLineZone; }

  bool RemovePoint(const QPointF& relPos);
  bool AddPoint(const QPointF& relPos);

  QPointF RelativePos(const QPoint& point);
  QPoint AbsolutPos(const QPointF& point);
  bool IsNear(const QPointF& point1, const QPointF& point2);
  bool IsZero(const QPointF& point);

  void Redraw();
  void DrawImpl();
  void DrawArea(QPainter* painter);
  void DrawLineZone(QPainter* painter);
  void DrawLine(QPainter* painter);
  void DrawPoints(QPainter* painter);

private slots:
  void Draw();

signals:
  void DoDraw();
  void NeedSave();

public:
  explicit ImageWithPoints(QWidget *parent = 0);
};

