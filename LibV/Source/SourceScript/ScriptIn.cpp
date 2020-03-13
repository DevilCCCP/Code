#include <QStringList>
#include <QImage>

#include <Lib/Log/Log.h>

#include "ScriptIn.h"


bool ScriptIn::Open(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QFile::ReadOnly)) {
    Log.Error(QString("File not opened (filename: '%1')").arg(filename));
    return false;
  }

  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    if (!line.startsWith('!')) {
      continue;
    }
    QByteArray section = line.trimmed();
    if (section.startsWith("!Header")) {
      if (!ParseHeader(file)) {
        Log.Error(QString("Parse header fail (filename: '%1')").arg(filename));
        return false;
      }
    } else if (section.startsWith("!Units")) {
      if (!ParseUnits(file)) {
        Log.Error(QString("Parse units fail (filename: '%1')").arg(filename));
        return false;
      }
    } else if (section.startsWith("!Points")) {
      if (!ParsePoints(file)) {
        Log.Error(QString("Parse points fail (filename: '%1')").arg(filename));
        return false;
      }
    }
  }

  InitScene();
  return true;
}

bool ScriptIn::GetNext(FrameS& frame)
{
  int frameFullSize = mWidth * mHeight * 3 / 2;
  frame = FrameS(new Frame());
  frame->ReserveData(frameFullSize);
  Frame::Header* header = frame->InitHeader();
  header->Timestamp = mSceneTime;
  header->Key = true;
  header->Compression = eRawNv12;
  header->Width = mWidth;
  header->Height = mHeight;
  header->Size = sizeof(Frame::Header) + frameFullSize;
  header->VideoDataSize = frameFullSize;
  memset(frame->VideoData(), 0, frameFullSize);

  mFrameData = (uchar*)frame->VideoData();
  for (auto itr = mUnitScripts.begin(); itr != mUnitScripts.end(); itr++) {
    Script* script = &*itr;
    MoveUnit(script);
  }
  mSceneTime += mScenePeriod;
  return true;
}

void ScriptIn::InitScene()
{
  mSceneTime = 0;
}

bool ScriptIn::ParseHeader(QFile& file)
{
  while (!file.atEnd()) {
    QByteArray header = file.readLine();
    if (header.startsWith('!')) {
      break;
    }
    QList<QByteArray> dim = header.split(';');
    if (dim.size() == 2) {
      mWidth = dim[0].trimmed().toInt();
      mHeight = dim[1].trimmed().toInt();
      Log.Info(QString("Scene info (w: %1, h: %2)").arg(mWidth).arg(mHeight));
    }
  }
  return mWidth > 0 && mHeight > 0;
}

bool ScriptIn::ParseUnits(QFile& file)
{
  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    if (line.startsWith('!')) {
      break;
    }
    QList<QByteArray> dim = line.split(';');
    if (dim.size() == 3) {
      int id = dim[0].trimmed().toInt();
      int period = dim[1].trimmed().toInt();
      QString filenames = QString::fromUtf8(dim[2].trimmed());
      AddUnit(id, period, filenames);
    }
  }
  return true;
}

bool ScriptIn::AddUnit(int id, int period, const QString& filenames)
{
  QStringList filenamesList = filenames.split(QChar(','), QString::SkipEmptyParts);
  if (filenamesList.isEmpty() || (filenamesList.size() > 1 && period <= 0)) {
    Log.Warning(QString("Add unit filenames fail (id: %1)").arg(id));
    return false;
  }
  QList<QImage> images;

  Unit unit;
  unit.Width = unit.Height = 1024;
  unit.Period = period;
  for (auto itr = filenamesList.begin(); itr != filenamesList.end(); itr++) {
    const QString& filename = *itr;
    QImage image(filename);
    if (image.isNull()) {
      Log.Warning(QString("Add unit image fail (id: %1, filename: '%2')").arg(id).arg(filename));
      continue;
    }
    unit.Width = qMin(unit.Width, image.width());
    unit.Height = qMin(unit.Height, image.height());
    images.append(image);
  }

  unit.Data.resize(images.size());
  int index = 0;
  for (auto itr = images.begin(); itr != images.end(); itr++, index++) {
    QByteArray& data = unit.Data[index];
    data.resize(unit.Width * unit.Height);
    const QImage& image = *itr;
    for (int j = 0; j < unit.Height; j++) {
      const QRgb* src = (const QRgb*)image.scanLine(j);
      uchar* dst = (uchar*)data.data() + j * unit.Width;
      for (int i = 0; i < unit.Width; i++) {
        *dst = qGray(*src);

        dst++;
        src++;
      }
    }
  }
  mUnitsMap[id] = unit;
  Log.Info(QString("Add unit (id: %1, imgs: %2)").arg(id).arg(unit.Data.size()));
  return true;
}

bool ScriptIn::ParsePoints(QFile& file)
{
  Script script;
  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    if (line.startsWith('!')) {
      break;
    }
    if (!script.UnitId) {
      script.UnitId = line.trimmed().toInt();
      continue;
    }
    Script::ScriptPoint point;
    QList<QByteArray> dim = line.split(';');
    if (dim.size() == 3) {
      point.Point.rx() = dim[0].trimmed().toInt();
      point.Point.ry() = dim[1].trimmed().toInt();
      point.Time = dim[2].trimmed().toInt();

      script.Points.append(point);
    }
  }

  if (script.UnitId && script.Points.size() >= 1) {
    mUnitScripts.append(script);
  }
  return true;
}

void ScriptIn::MoveUnit(Script* script)
{
  const Script::ScriptPoint& point1 = script->Points[script->Iteration];
  const Script::ScriptPoint& point2 = (script->Iteration + 1 < script->Points.size())? script->Points[script->Iteration + 1]: script->Points[0];
  QPoint p1(point1.Point.x() * mWidth / 100, point1.Point.y() * mHeight / 100);
  QPoint p2(point2.Point.x() * mWidth / 100, point2.Point.y() * mHeight / 100);
  QPoint p = (p1 * (point1.Time - script->ItrTime) + p2 * script->ItrTime) / point1.Time;

  DrawUnit(script->UnitId, p);

  script->ItrTime += mScenePeriod;
  if (script->ItrTime >= point1.Time) {
    script->ItrTime = 0;
    script->Iteration++;
    if (script->Iteration >= script->Points.size()) {
      script->Iteration = 0;
    }
  }
}

void ScriptIn::DrawUnit(int unitId, const QPoint& p)
{
  auto itr = mUnitsMap.find(unitId);
  if (itr == mUnitsMap.end()) {
    return;
  }

  Unit* unit = &itr.value();
  const QByteArray& img = unit->Data[unit->Iteration];
  DrawImage(img, unit->Width, unit->Height, p);

  unit->ItrTime += mScenePeriod;
  if (unit->ItrTime >= unit->Period) {
    unit->ItrTime = 0;
    unit->Iteration++;
    if (unit->Iteration >= unit->Data.size()) {
      unit->Iteration = 0;
    }
  }
}

void ScriptIn::DrawImage(const QByteArray& img, int width, int height, const QPoint& p)
{
  int js1 = 0;
  int js2 = height;
  int jd1 = p.y() - height/2;
  int jd2 = jd1 + height;
  if (jd2 < 0 || jd1 >= mHeight) {
    return;
  }

  int is1 = 0;
  int is2 = width;
  int id1 = p.x() - width/2;
  int id2 = id1 + width;
  if (id2 < 0 || id1 >= mWidth) {
    return;
  }

  if (jd1 < 0) {
    js1 -= jd1;
    jd1 = 0;
  } if (jd2 >= mHeight) {
    js2 -= jd2 - mHeight;
    jd2 = mHeight;
  }

  if (id1 < 0) {
    is1 -= id1;
    id1 = 0;
  } if (id2 >= mWidth) {
    is2 -= id2 - mWidth;
    id2 = mWidth;
  }

  int jd = jd1;
  for (int j = js1; j < js2; j++, jd++) {
    const uchar* src = (const uchar*)img.constData() + j * width + is1;
    uchar* dst = mFrameData + jd * mWidth + id1;
    for (int i = is1; i < is2; i++) {
      if (*src) {
        *dst = *src;
      }

      src++;
      dst++;
    }
  }
}


ScriptIn::ScriptIn()
  : mScenePeriod(50), mWidth(0), mHeight(0)
{
}

ScriptIn::~ScriptIn()
{
}
