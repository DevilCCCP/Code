#include <QtConcurrentRun>
#include <QtMath>
#include <QDateTime>
#include <QPixmap>
#include <QPainter>
#include <QBrush>
#include <QDebug>

#include <Lib/CoreUi/Version.h>
#include <Lib/Common/DMath.h>

#include "MapGenerator.h"

#define RandomSeed 0


const int kPreviewScale = 2;
const int kDeformCount = 10;
const int kMaxCutCount = 1000;
const qreal kDeformPlateValue = 0.05;

enum Stage {
  ePlateStage = 1,
//  eSuperPlateStage = 1,
//  ePlateCutStage,
//  ePlateMoveStage,
//  eCutWorldStage,
//  eHeightStage,
//  eWaterMoveStage,
  eUserStage,
  eFinished,
  eStartStage = ePlateStage,
  eFinalStage = eUserStage,
};

int MapGenerator::GetCurrentStage()
{
  return mCurrentStage;
}

int MapGenerator::GetCompleteStage()
{
  return mCompleteStage;
}

QString MapGenerator::GetStageName()
{
  switch (mCurrentStage) {
  case ePlateStage: return "Создание материков";
//  case eSuperPlateStage: return "Создание суперматерика";
//  case ePlateCutStage  : return "Отделение тектонических плит";
//  case ePlateMoveStage : return "Движение тектонических плит";
//  case eCutWorldStage  : return "Обрезка лишних частей";
//  case eHeightStage    : return "Создание ландшафта";
//  case eWaterMoveStage : return "Создание водоёмов и рек";
  case eUserStage      : return "Подтверждение пользователем";
  case eFinished       : return "Завершение";
  }
  return "<Некорректная операция>";
}

const EarthLandscape& MapGenerator::GetEarthLandscape()
{
  if (mGenerateWatcher->isRunning()) {
    return mEarthLandscape;
  }

  switch (mCurrentStage) {
  case ePlateStage     : CreatePreviewPlates(); break;
//  case eSuperPlateStage: CreatePreviewSuperPlate(); break;
//  case ePlateCutStage  : CreatePreviewStartPlates(); break;
//  case ePlateMoveStage : CreatePreviewMovedPlates(); break;
//  case eCutWorldStage  : CreatePreviewWorldPlates(); break;
//  case eHeightStage    : CreatePreviewHeights(); break;
//  case eWaterMoveStage : CreatePreviewWater(); break;
  case eUserStage      : CreatePreviewFinal(); break;
  case eFinished       : break;
  }
  return mEarthLandscape;
}

void MapGenerator::SetParameters(const MapParameters& mapParameters)
{
  mMapParameters = mapParameters;
  mAutoGenerate = false;
}

void MapGenerator::Start(const MapParameters& mapParameters, bool autoGenerate)
{
  if (mGenerateWatcher->isRunning()) {
    return;
  }

  mMapParameters = mapParameters;
  mAutoGenerate = autoGenerate;
  mCurrentStage = eStartStage;
  mCompleteStage = 0;
  mGeneratePercent = 0;

  CreateWorld();

  if (mAutoGenerate) {
    Generate();
  } else {
    emit Finished();
  }
}

void MapGenerator::Generate()
{
  if (mGenerateWatcher->isRunning()) {
    return;
  }

  mCancel = false;
  auto feature = QtConcurrent::run(this, &MapGenerator::DoGenerate, mAutoGenerate);
  mGenerateWatcher->setFuture(feature);
}

void MapGenerator::Cancel()
{
  mCancel = true;
}

void MapGenerator::Back()
{
  if (mGenerateWatcher->isRunning()) {
    return;
  }

  mCurrentStage = qMax(1, mCurrentStage - 1);
}

void MapGenerator::Forward()
{
  if (mGenerateWatcher->isRunning()) {
    return;
  }

  mCurrentStage = qMin(qMin(mStageMax, mCompleteStage + 1), mCurrentStage + 1);
}

void MapGenerator::Finish()
{
  if (mGenerateWatcher->isRunning()) {
    return;
  }

  mCurrentStage = eFinished;
}

bool MapGenerator::UpdatePercent(int perc)
{
  if (perc > mGeneratePercent) {
    mGeneratePercent = perc;
    emit Percent(mCurrentStage, mGeneratePercent);
  }
  return !mCancel;
}

void MapGenerator::DoGenerate(bool autoGenerate)
{
#ifdef RandomSeed
  mRand.seed(RandomSeed);
#else
  int seed = (int)QDateTime::currentMSecsSinceEpoch() % 1000;
  qDebug() << seed;
  mRand.seed(seed);
#endif

  if (autoGenerate) {
    while (mCurrentStage <= mStageMax) {
      DoGenerateStage();
      mCurrentStage++;
      mGeneratePercent = 0;
    }
  } else {
    DoGenerateStage();
  }
}

void MapGenerator::DoGenerateStage()
{
  mCompleteStage = mCurrentStage - 1;
  mGeneratePercent = 0;

  switch (mCurrentStage) {
  case ePlateStage     : CreatePlate(); break;
//  case eSuperPlateStage: CreateSuperPlate(); break;
//  case ePlateCutStage  : CutSuperPlate(); break;
//  case ePlateMoveStage : MovePlates(); break;
//  case eCutWorldStage  : CutWorldPlates(); break;
//  case eHeightStage    : CreateHeight(); break;
//  case eWaterMoveStage : MoveWater(); break;
  case eUserStage      : break;
  }

  mGeneratePercent = 100;
  mCompleteStage = mCurrentStage;
  emit Percent(mCurrentStage, mGeneratePercent);
}

void MapGenerator::CreateWorld()
{
}

void MapGenerator::CreatePlate()
{
  mMainHill.clear();
//  int count = mMapParameters.getWorldPlateCount();
//  for (int i = 0; i < count; i++) {
//    CreateNextPlate();
//  }
  CreateNextPlate();
}

void MapGenerator::CreateNextPlate()
{
  int perc = mRand.bounded(100);
  int partCount = perc < 10? 1: perc < 30? 2: 3;

}

void MapGenerator::CreateSuperPlate()
{
//  qreal h = 0.02 * getMapParameters().getGroundPercent();
//  qreal theta = qAcos(1 - h) * 180.0 / M_PI;

  QVector<QPointF> border;
  QPointF center(0, 0);
  for (int j = 18; j >= -18; j--) {
//    qreal alpha = j * M_PI / 18.0;
//    QPointF p = center + theta * QPointF(sin(alpha), cos(alpha));
    border.append(QPointF(j, 60));
  }

  mSuperPlate.Init(center, border);

//  qreal disp = kDeformPlateValue * qSqrt(mWorldPlate.Square());
//  for (int i = 0; i < 100; i++) {
//    if (!UpdatePercent(i)) {
//      break;
//    }
//    Plate deformPlate = mSuperPlate;
//    deformPlate.DeformPlate(disp, kDeformCount);
//    if (deformPlate.TestSelfIntersect()) {
//      mSuperPlate = deformPlate;
//      break;
//    }
//  }
}

void MapGenerator::CreateCentralPlate(Plate* plate, qreal scale, qreal sector)
{
  QPointF center(0, 0);
  QVector<QPointF> borderlt;
  QVector<QPointF> borderrt;
  QVector<QPointF> borderlb;
  QVector<QPointF> borderrb;
  int radius = mMapParameters.getGlobeRadius();
  qreal height = 0.5 * M_PI * radius;
  for (qreal h = 0; h < 0.9 * height * scale; h += 1) {
    qreal alpha = (h / scale) / radius;
    qreal d = radius * qCos(alpha);
    qreal w = sector * d * scale;
    borderlt.append(QPointF(-0.5*w, h));
    borderrt.prepend(QPointF(0.5*w, h));
    borderlb.prepend(QPointF(-0.5*w, -h));
    borderrb.append(QPointF(0.5*w, -h));
  }
  borderlb.removeLast();
  borderrt.removeLast();
  QPointF lt = borderlt.last();
  QPointF rt = borderrt.first();
  QPointF lb = borderlb.first();
  QPointF rb = borderrb.last();
  for (qreal p = lt.x() + 1; p < rt.x() - 1; p++) {
    borderlt.append(QPointF(p, lt.y()));
  }
  for (qreal p = rb.x() - 1; p > lb.x() + 1; p--) {
    borderrb.append(QPointF(p, rb.y()));
  }

  QVector<QPointF> border;
  border.append(borderlt);
  border.append(borderrt);
  border.append(borderrb);
  border.append(borderlb);
  plate->Init(center, border);
}

void MapGenerator::CreateNewCentralPlate(Plate* plate, qreal scale, qreal sectorLeft, qreal sectorRight)
{
  QPointF center(0, 0);
  QVector<QPointF> borderlt;
  QVector<QPointF> borderrt;
  QVector<QPointF> borderlb;
  QVector<QPointF> borderrb;
  int radius = mMapParameters.getGlobeRadius();
  qreal height = 0.5 * M_PI * radius;
  for (qreal h = 0; h < 0.9 * height * scale; h += 1) {
    qreal alpha = (h / scale) / radius;
    qreal d = radius * qCos(alpha);
    qreal wl = sectorLeft * d * scale;
    qreal wr = sectorRight * d * scale;
    borderlt.append(QPointF(wl, h));
    borderrt.prepend(QPointF(wr, h));
    borderlb.prepend(QPointF(wl, -h));
    borderrb.append(QPointF(wr, -h));
  }
  borderlb.removeLast();
  borderrt.removeLast();
  QPointF lt = borderlt.last();
  QPointF rt = borderrt.first();
  QPointF lb = borderlb.first();
  QPointF rb = borderrb.last();
  for (qreal p = lt.x() + 1; p < rt.x() - 1; p++) {
    borderlt.append(QPointF(p, lt.y()));
  }
  for (qreal p = rb.x() - 1; p > lb.x() + 1; p--) {
    borderrb.append(QPointF(p, rb.y()));
  }

  QVector<QPointF> border;
  border.append(borderlt);
  border.append(borderrt);
  border.append(borderrb);
  border.append(borderlb);
  plate->Init(center, border);
}

void MapGenerator::CutSuperPlate()
{
  mInitPlateList.clear();
  int count = mMapParameters.getGlobePlateCount();
  mCutCounter = 0;
  CutPlate(mSuperPlate, count);

  for (Plate& plate: mInitPlateList) {
    plate.CalcCenter();
  }
}

void MapGenerator::MovePlates()
{
  const qreal kFastMoveStep = 5.0;
  const qreal kSlowMoveStep = 1.0;
  const int kFastMoveCount = 5;
  const int kSlowMoveCount = 70;

  mMovedPlateList.clear();
  mMovedPlateList.resize(mInitPlateList.size());
  for (int i = 0; i < mInitPlateList.size(); i++) {
    mMovedPlateList[i] = mInitPlateList.at(i);
  }

  for (int i = 0; i < kFastMoveCount; i++) {
    if (!UpdatePercent(i * 100 / kSlowMoveCount)) {
      break;
    }
    MovePlatesOne(kFastMoveStep);
  }
  for (int i = kFastMoveCount; i < kSlowMoveCount; i++) {
    if (!UpdatePercent(i * 100 / kSlowMoveCount)) {
      break;
    }
    MovePlatesOne(kSlowMoveStep);
  }
  for (int i = 0; i < mMovedPlateList.size(); i++) {
    mMovedPlateList[i].Move();
    mMovedPlateList[i].CalcCenter();
  }
}

void MapGenerator::MovePlatesOne(qreal length)
{
  Q_UNUSED(length);
//  QVector<QPointF> step;
//  step.resize(mMovedPlateList.size());
//  step.fill(QPointF(0, 0));
//  for (int i = 0; i < mMovedPlateList.size(); i++) {
//    const Plate& plate1 = mMovedPlateList.at(i);
//    QPointF c1 = plate1.Center();
//    for (int j = 0; j < mMovedPlateList.size(); j++) {
//      if (i == j) {
//        continue;
//      }
//      const Plate& plate2 = mMovedPlateList.at(j);
//      qreal d = qMax(plate1.Distance(plate2), 1.0);
//      QPointF c2 = plate2.Center();
//      QPointF v = c1 - c2;
//      if (v.manhattanLength() > 0.000000000001) {
//        v /= qSquare(v);
//      }
//      step[i] += v / (d * d);
//    }
//    qreal d = qMax(plate1.Distance(mWorldPlate), 1.0);
//    if (d < 5) {
//      step[i] = QPointF(0, 0);
//    }
//    QPointF v = -c1;
//    if (v.manhattanLength() > 0.000000000001) {
//      v /= qSquare(v);
//    }
//    step[i] += v / (d * d);
//    if (step[i].manhattanLength() > 0.000000000001) {
//      step[i] = step[i] / qSquare(step[i]);
//    }
//    step[i] *= length;
//  }

//  for (int i = 0; i < mMovedPlateList.size(); i++) {
//    mMovedPlateList[i].Move(step.at(i));
//  }
}

void MapGenerator::CutPlate(const Plate& mainPlate, int count)
{
  if (count == 1) {
    mInitPlateList.append(mainPlate);
    return;
  }

  qreal totalSize = mainPlate.Square();
  int minDelta = qMin((int)(2.0 * qSqrt(totalSize) / (count)), mainPlate.Border().size() / 4);
  for (; mCutCounter < kMaxCutCount; mCutCounter++) {
    int perc = qRound(100.0 * mCutCounter / kMaxCutCount);
    if (!UpdatePercent(perc)) {
      break;
    }

    int i1 = 0;
    int i2 = mainPlate.Border().size()/2;
    for (int i = 0; i < 100; i++) {
      i1 = mRand.bounded(mainPlate.Border().size());
      i2 = mRand.bounded(mainPlate.Border().size());
      int iDelta1 = i1 - i2;
      int iDelta2 = i2 - i1;
      if (iDelta1 < 0) {
        iDelta1 += mainPlate.Border().size();
      } if (iDelta1 >= mainPlate.Border().size()) {
        iDelta1 -= mainPlate.Border().size();
      } if (iDelta2 < 0) {
        iDelta2 += mainPlate.Border().size();
      } if (iDelta2 >= mainPlate.Border().size()) {
        iDelta2 -= mainPlate.Border().size();
      }
      if (iDelta1 > minDelta && iDelta2 > minDelta) {
        break;
      }
    }

    Plate plate1, plate2;
    int count1 = 0;
    int count2 = 0;
    if (!mainPlate.CreateCut(&mRand, i1, i2, count, &plate1, &plate2, count1, count2)) {
      continue;
    }

    CutPlate(plate1, count1);
    CutPlate(plate2, count2);
    return;
  }

  mInitPlateList.append(mainPlate);
}

void MapGenerator::CutWorldPlates()
{
//  if (mMapParameters.getWorldPlateCount() >= mMapParameters.getGlobePlateCount()) {
//    return;
//  }

//  mWorldPlateList = mMovedPlateList;
//  while (mWorldPlateList.size() > mMapParameters.getWorldPlateCount()) {
//    int cutIndex = 0;
//    qreal mostRight = -mWidth;
//    for (int i = 0; i < mWorldPlateList.size(); i++) {
//      const Plate& plate = mWorldPlateList.at(i);
//      for (const QPointF& p: plate.Border()) {
//        if (p.x() > mostRight) {
//          mostRight = p.x();
//          cutIndex = i;
//        }
//      }
//    }
//    mWorldPlateList.removeAt(cutIndex);
//  }

//  int radius = mMapParameters.getGlobeRadius();
//  qreal leftAlpha = 2*mWidth;
//  qreal rightAlpha = -2*mWidth;
//  for (int i = 0; i < mWorldPlateList.size(); i++) {
//    const Plate& plate = mWorldPlateList.at(i);
//    for (const QPointF& p: plate.Border()) {
//      qreal alpha = p.y() / radius;
//      qreal d = radius * qCos(alpha);
//      qreal sector = p.x() / d;
//      leftAlpha = qMin(leftAlpha, sector);
//      rightAlpha = qMax(rightAlpha, sector);
//    }
//  }

//  qreal sector = mMapParameters.getGlobeSector() * M_PI / 180.0;
//  qreal leftBorder = -0.5 * sector;
//  qreal borderAlpha = leftAlpha - leftBorder;
//  leftAlpha -= borderAlpha;
//  rightAlpha += borderAlpha;
//  qreal rightBorder = 0.5 * sector;
//  qreal cutAlpha = rightBorder - rightAlpha;
//  sector -= cutAlpha;

//  CreateCentralPlate(&mNewWorldPlate, 1.0, sector);
//  for (Plate& plate: mWorldPlateList) {
//    plate.Rotate(radius, 0.5 * cutAlpha);
//  }
}

void MapGenerator::CreateHeight()
{

}

void MapGenerator::MoveWater()
{

}

void MapGenerator::CreatePreviewEmpty()
{
  mEarthLandscape.clear();
  mEarthLandscape.append(EarthLevel());
}

void MapGenerator::CreatePreviewPlates()
{
  CreatePreviewEmpty();

  EarthLevel* landLevel = &mEarthLandscape.front();
  for (const Hill& hill: qAsConst(mMainHill)) {
    EarthPlate earthPlate;
    earthPlate.Color = QColor(Qt::lightGray);
    earthPlate.Border = hill.High;
    landLevel->append(earthPlate);
  }

  mEarthLandscape.append(EarthLevel());
  landLevel = &mEarthLandscape.front();
  for (const Hill& hill: qAsConst(mMainHill)) {
    EarthPlate earthPlate;
    earthPlate.Color = QColor(Qt::darkYellow);
    earthPlate.Border = hill.Middle;
    landLevel->append(earthPlate);
  }

  mEarthLandscape.append(EarthLevel());
  landLevel = &mEarthLandscape.front();
  for (const Hill& hill: qAsConst(mMainHill)) {
    EarthPlate earthPlate;
    earthPlate.Color = QColor(Qt::darkGreen);
    earthPlate.Border = hill.Low;
    landLevel->append(earthPlate);
  }
}

void MapGenerator::CreatePreviewWorldPlate()
{
  CreatePreviewEmpty();
}

void MapGenerator::CreatePreviewSuperPlate()
{
  CreatePreviewEmpty();
  AddPlate(mSuperPlate, QColor(Qt::darkGreen));
}

void MapGenerator::CreatePreviewStartPlates()
{
  CreatePreviewEmpty();
  for (int i = 0; i < mInitPlateList.size(); i++) {
    const Plate& plate = mInitPlateList.at(i);
    int hue = 0 + (2*i + 1) * 300 / (2 * mInitPlateList.size());
    int saturation = 200;
    int value = 150;
    QColor plateColor;
    plateColor.setHsv(hue, saturation, value);
    AddPlate(plate, plateColor);
  }
}

void MapGenerator::CreatePreviewMovedPlates()
{
  CreatePreviewEmpty();
  for (int i = 0; i < mMovedPlateList.size(); i++) {
    const Plate& plate = mMovedPlateList.at(i);
    int hue = 0 + (2*i + 1) * 300 / (2 * mMovedPlateList.size());
    int saturation = 200;
    int value = 150;
    QColor plateColor;
    plateColor.setHsv(hue, saturation, value);
    AddPlate(plate, plateColor);
  }
}

void MapGenerator::CreatePreviewWorldPlates()
{
  CreatePreviewEmpty();
  for (int i = 0; i < mWorldPlateList.size(); i++) {
    const Plate& plate = mWorldPlateList.at(i);
    int hue = 0 + (2*i + 1) * 300 / (2 * mWorldPlateList.size());
    int saturation = 200;
    int value = 150;
    QColor plateColor;
    plateColor.setHsv(hue, saturation, value);
    AddPlate(plate, plateColor);
  }
}

void MapGenerator::CreatePreviewHeights()
{
  CreatePreviewEmpty();
}

void MapGenerator::CreatePreviewWater()
{
  CreatePreviewEmpty();
}

void MapGenerator::CreatePreviewFinal()
{
  CreatePreviewEmpty();
}

void MapGenerator::AddPlate(const Plate& plate, const QColor& color)
{
  if (mEarthLandscape.isEmpty()) {
    mEarthLandscape.append(EarthLevel());
  }

  EarthLevel* landLevel = &mEarthLandscape.front();
  EarthPlate earthPlate;
  earthPlate.Color = color;
  earthPlate.Border = plate.Border();
  landLevel->append(earthPlate);
}


MapGenerator::MapGenerator(QObject* parent)
  : QObject(parent)
  , mAutoGenerate(true), mStageMax(eFinalStage), mCurrentStage(1), mCompleteStage(0), mGeneratePercent(0)
  , mGenerateWatcher(new QFutureWatcher<void>(this)), mCancel(false)
{
  connect(mGenerateWatcher, &QFutureWatcher<void>::started, this, &MapGenerator::Started);
  connect(mGenerateWatcher, &QFutureWatcher<void>::finished, this, &MapGenerator::Finished);
}
