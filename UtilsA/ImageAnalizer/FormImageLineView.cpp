#include <QSettings>

#include "FormImageLineView.h"
#include "FormImageLineRegion.h"


const QVector<QPoint> FormImageLineView::LinePoints() const
{
  return mFormImageLineRegion->LinePoints();
}

const QList<uchar> FormImageLineView::LineValues() const
{
  return mFormImageLineRegion->LineValues();
}

const QVector<uchar> FormImageLineView::RectValues() const
{
  return mFormImageLineRegion->RectValues();
}

void FormImageLineView::SetMode(FormImageLineView::EMode _Mode)
{
  switch (mMode) {
  case eSelect   : break;
  case eLine     : mLinePoints = mFormImageLineRegion->getLinePoints(); break;
  case eRectangle: mRectPoints = mFormImageLineRegion->getLinePoints(); break;
  }

  mMode = _Mode;

  switch (mMode) {
  case eSelect   : mFormImageLineRegion->SetNone(); break;
  case eLine     : mFormImageLineRegion->SetLine(mLinePoints); break;
  case eRectangle: mFormImageLineRegion->SetRegion(mRectPoints); break;
  }
}

void FormImageLineView::SyncSettings(QSettings* settings)
{
//  settings->setValue("Mode", (int)mMode);
  switch (mMode) {
  case eSelect   : break;
  case eLine     : mLinePoints = mFormImageLineRegion->getLinePoints(); break;
  case eRectangle: mRectPoints = mFormImageLineRegion->getLinePoints(); break;
  }

  SyncLine(settings);
  SyncRect(settings);
  settings->sync();
}

void FormImageLineView::RestoreSettings(QSettings* settings)
{
  mMode = eSelect;

  RestoreLine(settings);
  RestoreRect(settings);
}

void FormImageLineView::MoveLineX(int x)
{
  mFormImageLineRegion->MoveLineX(x);
}

void FormImageLineView::MoveLineY(int y)
{
  mFormImageLineRegion->MoveLineY(y);
}

QString FormImageLineView::PointVarname(int i) const
{
  return QString("Line %1").arg(i);
}

void FormImageLineView::SyncLine(QSettings* settings) const
{
  settings->beginGroup("LineView");
  for (int i = 0; i < mLinePoints.size(); i++) {
    settings->setValue(PointVarname(i), mLinePoints.at(i));
  }
  for (int i = mLinePoints.size(); !settings->value(PointVarname(i)).isNull(); i++) {
    settings->setValue(PointVarname(i), QVariant());
  }
  settings->endGroup();
}

void FormImageLineView::SyncRect(QSettings* settings) const
{
  settings->beginGroup("RectView");
  for (int i = 0; i < mRectPoints.size(); i++) {
    settings->setValue(PointVarname(i), mRectPoints.at(i));
  }
  for (int i = mRectPoints.size(); !settings->value(PointVarname(i)).isNull(); i++) {
    settings->setValue(PointVarname(i), QVariant());
  }
  settings->endGroup();
}

void FormImageLineView::RestoreLine(QSettings* settings)
{
  settings->beginGroup("LineView");
  for (int i = 0; ; i++) {
    QVariant v = settings->value(PointVarname(i));
    if (v.isNull()) {
      break;
    }
    QPointF p = v.toPointF();
    mLinePoints.append(p);
  }
  settings->endGroup();
}

void FormImageLineView::RestoreRect(QSettings* settings)
{
  settings->beginGroup("RectView");
  for (int i = 0; ; i++) {
    QVariant v = settings->value(PointVarname(i));
    if (v.isNull()) {
      break;
    }
    QPointF p = v.toPointF();
    mRectPoints.append(p);
  }
  settings->endGroup();
}


FormImageLineView::FormImageLineView(QWidget* parent)
  : FormImageView(parent)
  , mFormImageLineRegion(new FormImageLineRegion()), mMode(eSelect)
{
  SetImageRegion(mFormImageLineRegion);

  connect(mFormImageLineRegion, &FormImageLineRegion::LineChanged, this, &FormImageLineView::LineChanged);
}
