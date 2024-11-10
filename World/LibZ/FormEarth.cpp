#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QDateTime>
#include <QtMath>
#include <QDebug>

#include "FormEarth.h"
#include "ui_FormEarth.h"


const int kControlMargin = 12;
const int kMoveThreshold = 5;
const qreal kThetaMoveStep = 0.25;
const qreal kPhiMoveStep = 1.0;
const qreal kCamViewAngel = 0.3 * M_PI;
int gDebug = 0;

FormEarth::FormEarth(QWidget* parent)
  : QWidget(parent), ui(new Ui::FormEarth)
  , mShowAxes(true), mMinScale(true)
  , mDefaultCamTheta(90.0), mDefaultCamPhi(-90.0), mDefaultCamH(1.0)
  , mMainColor(QColor(Qt::darkBlue)), mAxeColor(QColor(Qt::darkGray))
  , mCamTheta(90.0), mCamPhi(-90.0), mCamH(1.0), mZoomDelta(0), mMoving(false)
  , mUpdated(false)
//  , mCamTheta(90.0), mCamPhi(0.0), mCamH(0.003), mZoomDelta(0), mMoving(false)
{
  Q_INIT_RESOURCE(LibZ);

  ui->setupUi(this);

  ui->toolButtonZoomIn->setDefaultAction(ui->actionZoomIn);
  ui->toolButtonZoomOut->setDefaultAction(ui->actionZoomOut);
  ui->toolButtonZoomDefault->setDefaultAction(ui->actionZoomDefault);
  ui->toolButtonMoveLeft->setDefaultAction(ui->actionMoveLeft);
  ui->toolButtonMoveRight->setDefaultAction(ui->actionMoveRight);
  ui->toolButtonMoveUp->setDefaultAction(ui->actionMoveUp);
  ui->toolButtonMoveDown->setDefaultAction(ui->actionMoveDown);

//  addAction(ui->actionZoomIn);
//  addAction(ui->actionZoomOut);
//  addAction(ui->actionZoomDefault);
//  addAction(ui->actionMoveLeft);
//  addAction(ui->actionMoveRight);
//  addAction(ui->actionMoveUp);
//  addAction(ui->actionMoveDown);
//  setContextMenuPolicy(Qt::ActionsContextMenu);

  UpdatePrepare(100, 100);
}

FormEarth::~FormEarth()
{
  delete ui;
}


void FormEarth::resizeEvent(QResizeEvent* event)
{
  int w = event->size().width();
  int h = event->size().height();
  ui->widgetControls->move(w - ui->widgetControls->width() - kControlMargin, kControlMargin);

  UpdatePrepare(w, h);

  update();
}

void FormEarth::mouseMoveEvent(QMouseEvent* event)
{
  if (!mMoving) {
    return;
  }

  QPoint newMoved = event->pos() - mMoveAnchor;
  newMoved.rx() /= kMoveThreshold;
  if (newMoved != mMoved) {
    mMoved = newMoved;
    qreal scale = mCamH;
    UpdateValue(mCamTheta, qBound(10.0, mCamAnchor.x() - mMoved.y() * scale * kThetaMoveStep, 170.0));
    MovePhi(mCamAnchor.y() + mMoved.x() * scale * kPhiMoveStep);

    Update();
  }
}

void FormEarth::mousePressEvent(QMouseEvent* event)
{
  mZoomDelta = 0;
  if (event->buttons() == Qt::LeftButton) {
    mCamAnchor = QPointF(mCamTheta, mCamPhi);
    mMoveAnchor = event->pos();
    mMoving = true;
  } else if (mMoving) {
    UpdateValue(mCamTheta, mCamAnchor.x());
    UpdateValue(mCamPhi, mCamAnchor.y());
    mMoving = false;

    Update();

    event->accept();
  }
}

void FormEarth::mouseReleaseEvent(QMouseEvent* event)
{
  if (!event->buttons().testFlag(Qt::LeftButton) && mMoving) {
    mMoving = false;
  }
}

void FormEarth::wheelEvent(QWheelEvent* event)
{
  mZoomDelta += event->angleDelta().y();
  if (qAbs(mZoomDelta) >= 120) {
    int r = mZoomDelta / 120;
    mZoomDelta -= r * 120;
    Zoom(r);
  }
}

void FormEarth::paintEvent(QPaintEvent* event)
{
  if (mUpdated) {
    Draw();
    mUpdated = false;
  }

  QPainter painter(this);
  painter.drawPixmap(event->rect().topLeft(), mBackBuffer, event->rect());
}

void FormEarth::Update()
{
  if (!mUpdated) {
    return;
  }

  UpdatePrepare(size().width(), size().height());

  qDebug() << mCamTheta << mCamPhi << mCamH;
  update();
}

void FormEarth::UpdatePrepare(int w, int h)
{
  qreal screen = mMinScale? qMin(w, h): qMax(w, h);
  qreal d = screen / qTan(kCamViewAngel);
  UpdateValue(mPlaneScale, d);
  UpdateValue(mCenter.rx(), 0.5 * w);
  UpdateValue(mCenter.ry(), 0.5 * h);

  mZ0 = 1.0 / (1.0 + mCamH);
  qreal r = sqrt(1.0 - mZ0 * mZ0);
  mPlaneRadius = mPlaneScale * r / (mCamH + 1.0 - mZ0);
  mPlaneBorder.clear();
  const int kBorderPrecission = 1;
  for (int i = 0; i < kBorderPrecission*360; i++) {
    qreal alpha = i * (M_PI / (kBorderPrecission*180.0));
    mPlaneBorder.append(mCenter + QPointF(-cos(alpha) * mPlaneRadius, sin(alpha) * mPlaneRadius));
  }

  mAxeThetaD = 10;
  mAxePhiD = 10;
  mThetaD = 1.0;
  mPhiD = 1.0;
}

void FormEarth::UpdateValue(int& value, int newValue)
{
  if (value != newValue) {
    value = newValue;
    mUpdated = true;
  }
}

void FormEarth::UpdateValue(qreal& value, qreal newValue)
{
  if (value != newValue) {
    value = newValue;
    mUpdated = true;
  }
}

void FormEarth::Draw()
{
  if (mBackBuffer.isNull() || mBackBuffer.width() != width() || mBackBuffer.height() != height()) {
    mBackBuffer = QPixmap(width(), height());
  }

  QPainter painter;
  painter.begin(&mBackBuffer);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
  painter.fillRect(rect(), Qt::black);

  DrawBackground(&painter);
  DrawLandscape(&painter, mLandscape);
  if (mShowAxes) {
    DrawAxes(&painter);
  }
  painter.end();
}

void FormEarth::DrawBackground(QPainter* painter)
{
  painter->setPen(QPen(QBrush(Qt::black), 1.0));
  painter->setBrush(QBrush(mMainColor));

  painter->drawEllipse(mCenter, mPlaneRadius, mPlaneRadius);
}

static int gDrawPlateCounter = 0;
void FormEarth::DrawLandscape(QPainter* painter, const EarthLandscape& landscape)
{
  gDrawPlateCounter = 0;
  for (const EarthLevel& level: landscape) {
    for (const EarthPlate& plate: level) {
      painter->setPen(QPen(plate.Color));
      painter->setBrush(QBrush(plate.Color));

      DrawPlate(painter, plate.Border);
    }
  }
}

void FormEarth::DrawPlate(QPainter* painter, const QVector<QPointF>& border)
{
  if (border.size() <= 2) {
    return;
  }

  QList<QVector<QPointF> > arcList;
  QVector<QPointF>* currentArc = nullptr;
  int indexFirst = -1;
  int indexLast = -1;
  bool hasVisible = false;
  QPoint rotateBorder(0, 0);
  for (int i = 0; i < border.size(); i++) {
    const QPointF& p = border.at(i);
    QPointF ps;
    if (TranslateToPlaneRotate(p, ps, rotateBorder)) {
      hasVisible = true;
      if (indexFirst < 0) {
        indexFirst = i;
      }
      indexLast = i;
      if (!currentArc) {
        arcList.append(QVector<QPointF>());
        currentArc = &arcList.last();
      }
      currentArc->append(ps);
    } else {
      if (currentArc) {
        currentArc = nullptr;
      }
    }
  }

  if (!hasVisible) {
//    if ((rotateBorder.x() % 2) == 0 && rotateBorder.x() > 0) {
//      painter->drawPolygon(mPlaneBorder.constData(), mPlaneBorder.size());
//    }
    return;
  }

  bool hasInvisible = arcList.size() > 1 || indexFirst != 0 || indexLast != border.size() - 1;
  if (arcList.size() > 1 && indexFirst == 0 && indexLast == border.size() - 1) {
    arcList.last().append(arcList.first());
    arcList.removeFirst();
  }

  QList<QVector<QPointF> > sborderList;
  if (!hasInvisible) {
    sborderList.append(arcList.first());
    arcList.clear();
  }

  QMultiMap<int, const QVector<QPointF>*> arcStartMap;
  for (int i = 0; i < arcList.size(); i++) {
    const QVector<QPointF>* arc = &arcList.at(i);
    int firstIndex = FindBorderIndex(arc->first());
    arcStartMap.insert(firstIndex, arc);
  }
  while (!arcStartMap.isEmpty()) {
    const QVector<QPointF>* arc = arcStartMap.first();
    int firstIndex = arcStartMap.firstKey();
    int lastIndex = FindBorderIndex(arc->last());
    QVector<QPointF> sborder;
    sborder.append(*arc);
    arcStartMap.erase(arcStartMap.begin());
    while (!arcStartMap.isEmpty()) {
      auto itr = arcStartMap.lowerBound(lastIndex);
      if (itr == arcStartMap.end()) {
        if (firstIndex > lastIndex) {
          break;
        }
        itr = arcStartMap.lowerBound(0);
        if (itr == arcStartMap.end() || itr.key() > firstIndex) {
          break;
        }
      } else {
        if (firstIndex > lastIndex && itr.key() > firstIndex) {
          break;
        }
      }
      int nextIndex = itr.key();

      const QVector<QPointF>* nextArc = itr.value();
      int nextLastIndex = FindBorderIndex(nextArc->last());
      PlateConnectBorder(lastIndex, nextIndex, sborder);
      sborder.append(*nextArc);
      lastIndex = nextLastIndex;
      arcStartMap.erase(itr);
    }
    PlateConnectBorder(lastIndex, firstIndex, sborder);
    sborderList.append(sborder);
  }

  for (const QVector<QPointF>& sborder: qAsConst(sborderList)) {
    if (sborder.size() >= 2) {
      painter->drawPolygon(sborder.constData(), sborder.size());
    }
  }
}

void FormEarth::PlateConnectBorder(int fromIndex, int toIndex, QVector<QPointF>& sborder)
{
  int delta = toIndex - fromIndex;
  if (delta < 0) {
    delta += mPlaneBorder.size();
  }
  if (delta > mPlaneBorder.size()/2) {
    return;
  }
  if (fromIndex <= toIndex) {
    for (int j = fromIndex; j < toIndex; j++) {
      sborder.append(mPlaneBorder.at(j));
    }
  } else {
    for (int j = fromIndex; j < mPlaneBorder.size(); j++) {
      sborder.append(mPlaneBorder.at(j));
    }
    for (int j = 0; j < toIndex; j++) {
      sborder.append(mPlaneBorder.at(j));
    }
  }
}

int FormEarth::FindBorderIndex(const QPointF& p)
{
  int l = 0;
  int r = mPlaneBorder.size() - 1;
  qreal lV = (p - mPlaneBorder.at(l)).manhattanLength();
  qreal rV = (p - mPlaneBorder.at(r)).manhattanLength();
  while (l < r) {
    if (lV <= rV) {
      int m = (l + r) / 2;
      qreal mV = (p - mPlaneBorder.at(m)).manhattanLength();
      r = m;
      rV = mV;
    } else {
      int m = (l + r + 1) / 2;
      qreal mV = (p - mPlaneBorder.at(m)).manhattanLength();
      l = m;
      lV = mV;
    }
  }
  return l;
}

void FormEarth::DrawAxes(QPainter* painter)
{
  painter->setPen(QPen(QBrush(mAxeColor), 0.5));
  for (qreal theta = -90 + mAxeThetaD; theta <= 0; theta += mAxeThetaD) {
    QPointF lastPoint;
    bool isLastValid = false;
    for (qreal phi = 0; phi <= 360; phi += mPhiD) {
      QPointF nextPoint;
      bool isNextValid = TranslateToPlane(QPointF(phi, theta), nextPoint);
      if (isNextValid && isLastValid) {
        painter->drawLine(lastPoint, nextPoint);
      }
      isLastValid = isNextValid;
      lastPoint = nextPoint;
    }
  }

  for (qreal theta = 90 - mAxeThetaD; theta >= 0; theta -= mAxeThetaD) {
    QPointF lastPoint;
    bool isLastValid = false;
    for (qreal phi = 0; phi <= 360; phi += mPhiD) {
      QPointF nextPoint;
      bool isNextValid = TranslateToPlane(QPointF(phi, theta), nextPoint);
      if (isNextValid && isLastValid) {
        painter->drawLine(lastPoint, nextPoint);
      }
      isLastValid = isNextValid;
      lastPoint = nextPoint;
    }
  }

  for (qreal phi = 0; phi <= 360; phi += mAxePhiD) {
    QPointF lastPoint;
    bool isLastValid = false;
    for (qreal theta = -90; theta <= 90; theta += mThetaD) {
      QPointF nextPoint;
      bool isNextValid = TranslateToPlane(QPointF(phi, theta), nextPoint);
      if (isNextValid && isLastValid) {
        painter->drawLine(lastPoint, nextPoint);
      }
      isLastValid = isNextValid;
      lastPoint = nextPoint;
    }
  }
}

bool FormEarth::TranslateToPlane(const QPointF& p, QPointF& ps)
{
  qreal phi = p.x() * M_PI / 180.0;
  qreal theta = (90 - p.y()) * M_PI / 180.0;

  qreal x1 = sin(theta)*cos(phi);
  qreal y1 = sin(theta)*sin(phi);
  qreal z1 = cos(theta);

  qreal camPhi = -mCamPhi * M_PI / 180.0;
  qreal x2 = x1*cos(camPhi) - y1*sin(camPhi);
  qreal y2 = x1*sin(camPhi) + y1*cos(camPhi);
  qreal z2 = z1;

  qreal camTheta = -mCamTheta * M_PI / 180.0;
//  qreal x1 = x*cos(camTheta) + z*sin(camTheta);
//  qreal y1 = y;
//  qreal z1 = -x*sin(camTheta) + z*cos(camTheta);
  qreal x = x2;
  qreal y = y2*cos(camTheta) + z2*sin(camTheta);
  qreal z = -y2*sin(camTheta) + z2*cos(camTheta);

  if (z > mZ0) {
    ps.rx() = mCenter.x() + mPlaneScale * x / (mCamH + 1.0 - z);
    ps.ry() = mCenter.y() + mPlaneScale * y / (mCamH + 1.0 - z);
    return true;
  }

  return false;
}

bool FormEarth::TranslateToPlaneRotate(const QPointF& p, QPointF& ps, QPoint& rotate)
{
  qreal phi = p.x() * M_PI / 180.0;
  qreal theta = (90 - p.y()) * M_PI / 180.0;

  qreal x1 = sin(theta)*cos(phi);
  qreal y1 = sin(theta)*sin(phi);
  qreal z1 = cos(theta);

  qreal camPhi = -mCamPhi * M_PI / 180.0;
  qreal x2 = x1*cos(camPhi) - y1*sin(camPhi);
  qreal y2 = x1*sin(camPhi) + y1*cos(camPhi);
  qreal z2 = z1;

  qreal camTheta = -mCamTheta * M_PI / 180.0;
//  qreal x1 = x*cos(camTheta) + z*sin(camTheta);
//  qreal y1 = y;
//  qreal z1 = -x*sin(camTheta) + z*cos(camTheta);
  qreal x = x2;
  qreal y = y2*cos(camTheta) + z2*sin(camTheta);
  qreal z = -y2*sin(camTheta) + z2*cos(camTheta);

  if (y > 0) {
    if (rotate.y() < 0) {
      rotate.rx() += (x < 0? 1: -1);
    }
    rotate.ry() = 1;
  } else if (y < 0) {
    if (rotate.y() > 0) {
      rotate.rx() += (x > 0? 1: -1);
    }
    rotate.ry() = -1;
  }
  if (z > mZ0) {
    ps.rx() = mCenter.x() + mPlaneScale * x / (mCamH + 1.0 - z);
    ps.ry() = mCenter.y() + mPlaneScale * y / (mCamH + 1.0 - z);
    return true;
  }

  return false;
}

void FormEarth::Zoom(int k)
{
  qreal newCamH = mCamH;
  for (; k > 0; k--) {
    newCamH -= qMin(0.5 * newCamH, 1.0);
  }
  for (; k < 0; k++) {
    newCamH += qMin(newCamH, 1.0);
  }
  newCamH = qBound(0.0001, newCamH, 2.0);
  UpdateValue(mCamH, newCamH);

  Update();
}

void FormEarth::MovePhi(qreal newCamPhi)
{
  if (newCamPhi >= 360) {
    newCamPhi -= 360;
  }
  if (newCamPhi < 0) {
    newCamPhi += 360;
  }
  UpdateValue(mCamPhi, newCamPhi);

  Update();
}

void FormEarth::on_actionZoomIn_triggered()
{
  Zoom(1);
}

void FormEarth::on_actionZoomOut_triggered()
{
  Zoom(-1);
}

void FormEarth::on_actionZoomDefault_triggered()
{
  UpdateValue(mCamH, mDefaultCamH);
  UpdateValue(mCamTheta, mDefaultCamTheta);
  UpdateValue(mCamPhi, mDefaultCamPhi);

  Update();
}

void FormEarth::on_actionMoveLeft_triggered()
{
  MovePhi(mCamPhi + 10.0);
}

void FormEarth::on_actionMoveRight_triggered()
{
  MovePhi(mCamPhi - 10.0);
}

void FormEarth::on_actionMoveUp_triggered()
{
  UpdateValue(mCamTheta, qMax(10.0, mCamTheta - 10.0));

  Update();
}

void FormEarth::on_actionMoveDown_triggered()
{
  UpdateValue(mCamTheta, qMin(170.0, mCamTheta + 10.0));

  Update();
}

void FormEarth::on_toolButtonMoveDebug_clicked()
{
  mCamTheta = 53.25;
  mCamPhi   = 63;
  mCamH     = 1;
  mUpdated  = true;
  gDebug = 1;

  Update();
}
