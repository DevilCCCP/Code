#include <QSettings>
#include <QElapsedTimer>
#include <QSettings>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Common/Format.h>
#include <Lib/Log/Log.h>
#include <LibV/Storage/Container.h>


class TestMe {
  QElapsedTimer mTestTimer;

public:
  void StartTest(const QString& info)
  {
    Log.Info(QString(" --------- Start test: %1 --------- ").arg(info));
    mTestTimer.start();
  }

  void EndTest(bool ok)
  {
    qint64 ts = mTestTimer.elapsed();
    Log.Info(QString(" ========= %1 in %2 ========= ").arg(ok? "Done": "Fail").arg(FormatTimeDelta(ts)));
  }

  void EndTest(bool ok, const QString& info)
  {
    qint64 ts = mTestTimer.elapsed();
    Log.Info(QString(" ========= %1 in %2: %3 ========= ").arg(ok? "Done": "Fail").arg(FormatTimeDelta(ts)).arg(info));
  }

  void EndTest(int count)
  {
    qint64 ts = mTestTimer.elapsed();
    qreal countPerSec = 1000.0 * count / ts;
    Log.Info(QString(" ========= count: %1; time: %2; per sec: %3 ========= ").arg(count).arg(FormatTimeDelta(ts)).arg(countPerSec));
  }

  void EndTest(int count1, int count2)
  {
    qint64 ts = mTestTimer.elapsed();
    qreal countPerSec1 = 1000.0 * count1 / ts;
    qreal countPerSec2 = 1000.0 * count2 / ts;
    Log.Info(QString(" ========= count: %1/%2; time: %3; per sec: %4/%5 ========= ").arg(count1).arg(count2).arg(FormatTimeDelta(ts)).arg(countPerSec1).arg(countPerSec2));
  }

  void EndTest(int count1, int count2, int bytes)
  {
    qint64 ts = mTestTimer.elapsed();
    qreal countPerSec1 = 1000.0 * count1 / ts;
    qreal countPerSec2 = 1000.0 * count2 / ts;
    qreal bytesPerSec = 1000.0 * bytes / ts;
    Log.Info(QString(" ========= count: %1/%2 %3; time: %4; per sec: %5/%6 %7 ========= ")
             .arg(count1).arg(count2).arg(FormatBytes(bytes)).arg(FormatTimeDelta(ts))
             .arg(countPerSec1).arg(countPerSec2).arg(FormatBytes(bytesPerSec)));
  }
};

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  QString settingFile = qApp->applicationDirPath() + "/Var/store_test.ini";
  if (!QFile::exists(settingFile)) {
    Log.Fatal(QString("No storage settings file (path: '%1')").arg(settingFile));
    return -1;
  }
  QSettings settings(settingFile, QSettings::IniFormat);
  QString storePath = settings.value("Path").toString();
  int storeCell     = settings.value("CellSize", 16777216).toInt();
  int storePage     = settings.value("PageSize", 1048576).toInt();
  int storeCapacity = settings.value("Capacity", 200).toInt();

  Container container(storePath, storeCell, storePage, storeCapacity);
  if (!container.ConnectDig()) {
    Log.Fatal(QString("Connect storage fail"));
    return -2;
  }

  TestMe testMe;
  testMe.StartTest("1'st cell");
  if (container.OpenRead(1, 0)) {
    FrameS frame;
    int count = 0;
    while (container.ReadNextFrame(frame) && frame) {
      count++;
    }
    testMe.EndTest(count, QString("read %1 frames").arg(count));
  } else {
    testMe.EndTest(false);
  }

  const int kCellsTest = 50;
  testMe.StartTest(QString("1'st %1 cells").arg(kCellsTest));
  int countCell = 0;
  int countFrame = 0;
  int countBytes = 0;
  for (int i = 0; i < kCellsTest; i++) {
    if (container.OpenRead(i + 1, 0)) {
      countCell++;
      FrameS frame;
      while (container.ReadNextFrame(frame) && frame) {
        countFrame++;
        countBytes += frame->Size();
      }
    }
  }
  testMe.EndTest(countCell, countFrame, countBytes);

  qsrand(QDateTime::currentMSecsSinceEpoch());
  int startCell = rand() % (qMax(1, storeCapacity - kCellsTest));
  testMe.StartTest(QString("%1 straight from rand start (%2) cells").arg(kCellsTest).arg(startCell));
  countCell = 0;
  countFrame = 0;
  countBytes = 0;
  for (int i = 0; i < kCellsTest; i++) {
    if (container.OpenRead(startCell + i + 1, 0)) {
      countCell++;
      FrameS frame;
      while (container.ReadNextFrame(frame) && frame) {
        countFrame++;
        countBytes += frame->Size();
      }
    }
  }
  testMe.EndTest(countCell, countFrame, countBytes);

  testMe.StartTest(QString("%1 rand cells").arg(kCellsTest));
  countCell = 0;
  countFrame = 0;
  countBytes = 0;
  for (int i = 0; i < kCellsTest; i++) {
    int startCell = rand() % (qMax(1, storeCapacity - 1));
    if (container.OpenRead(startCell + 1, 0)) {
      countCell++;
      FrameS frame;
      while (container.ReadNextFrame(frame) && frame) {
        countFrame++;
        countBytes += frame->Size();
      }
    }
  }
  testMe.EndTest(countCell, countFrame, countBytes);

  return 0;
}

