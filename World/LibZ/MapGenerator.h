#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QFutureWatcher>
#include <QRandomGenerator>
#include <QPixmap>

#include <Lib/Include/Common.h>

#include "MapParameters.h"
#include "EarthLandscape.h"
#include "Plate.h"


class MapGenerator: public QObject
{
  PROPERTY_GET(MapParameters, MapParameters)
  PROPERTY_GET(bool,          AutoGenerate)
  PROPERTY_GET(int,           StageMax)
  volatile int                mCurrentStage;
  volatile int                mCompleteStage;
  int                         mGeneratePercent;

  QFutureWatcher<void>*       mGenerateWatcher;
  volatile bool               mCancel;

  QRandomGenerator            mRand;
  Plate                       mSuperPlate;
  QVector<Plate>              mInitPlateList;
  QVector<Plate>              mMovedPlateList;
  QVector<Plate>              mWorldPlateList;
  int                         mCutCounter;

  struct Hill {
    QVector<QPointF> Main;
    QVector<QPointF> High;
    QVector<QPointF> Middle;
    QVector<QPointF> Low;
  };
  typedef QVector<Hill> HillList;

  HillList                    mMainHill;
  EarthLandscape              mEarthLandscape;

  Q_OBJECT
  ;
public:
  int GetCurrentStage();
  int GetCompleteStage();
  QString GetStageName();
  const EarthLandscape& GetEarthLandscape();

public:
  void SetParameters(const MapParameters& mapParameters);
  void Start(const MapParameters& mapParameters, bool autoGenerate);
  void Generate();
  void Cancel();
  void Back();
  void Forward();
  void Finish();

protected:
  bool UpdatePercent(int perc);

private:
  void DoGenerate(bool autoGenerate);
  void DoGenerateStage();

  void CreateWorld();
  void CreatePlate();
  void CreateNextPlate();
  void CreateSuperPlate();
  void CreateCentralPlate(Plate* plate, qreal scale, qreal sector);
  void CreateNewCentralPlate(Plate* plate, qreal scale, qreal sectorLeft, qreal sectorRight);
  void CutSuperPlate();
  void MovePlates();
  void MovePlatesOne(qreal length);
  void CutPlate(const Plate& mainPlate, int count);
  void CutWorldPlates();
  void CreateHeight();
  void MoveWater();

  void CreatePreviewEmpty();
  void CreatePreviewPlates();
  void CreatePreviewWorldPlate();
  void CreatePreviewSuperPlate();
  void CreatePreviewStartPlates();
  void CreatePreviewMovedPlates();
  void CreatePreviewWorldPlates();
  void CreatePreviewHeights();
  void CreatePreviewWater();
  void CreatePreviewFinal();
  void AddPlate(const Plate& plate, const QColor& color);

signals:
  void Started();
  void Finished();
  void Percent(int stage, int percent);

public:
  MapGenerator(QObject* parent = nullptr);
};
