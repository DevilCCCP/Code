#pragma once

#include <QMap>
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QFile>
#include <QList>

#include <LibV/Include/Frame.h>

DefineClassS(ScriptIn);

class ScriptIn
{
  struct Unit {
    QVector<QByteArray> Data;
    int                 Width;
    int                 Height;
    int                 Period;

    int                 Iteration;
    int                 ItrTime;

    Unit(): Iteration(0), ItrTime(0) { }
  };

  struct Script {
    struct ScriptPoint {
      QPoint Point;
      int    Time;
    };

    int                UnitId;
    QList<ScriptPoint> Points;

    int                Iteration;
    int                ItrTime;

    Script(): UnitId(0), Iteration(0), ItrTime(0) { }
  };

  int             mScenePeriod;
  int             mWidth;
  int             mHeight;
  QMap<int, Unit> mUnitsMap;
  QList<Script>   mUnitScripts;

  int             mSceneTime;
  uchar*          mFrameData;

public:
  bool Open(const QString& filename);
  bool GetNext(FrameS& frame);

private:
  void InitScene();
  bool ParseHeader(QFile& file);
  bool ParseUnits(QFile& file);
  bool AddUnit(int id, int period, const QString& filenames);
  bool ParsePoints(QFile& file);

  void MoveUnit(Script* script);
  void DrawUnit(int unitId, const QPoint& p);
  void DrawImage(const QByteArray& img, int width, int height, const QPoint& p);

public:
  ScriptIn();
  ~ScriptIn();
};

