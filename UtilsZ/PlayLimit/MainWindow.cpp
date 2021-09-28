#include <QApplication>
#include <QDir>
#include <QDate>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QImage>
#include <QCloseEvent>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Format.h"


const int kDefaultCommonLimit = 2*60*60*1000;
const int kDefaultRedLimit = 4*60*60*1000;
const int kCalcPeriodMs = 60*1000;
const int kWarningTimeMs = 5 * 60 * 1000;
const int kWarningMs = 60 * 1000;
const int kFatalMs = 45 * 1000;

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow)
  , mTrayIcon(new QSystemTrayIcon(this))
  , mCalcTimer(new QTimer(this))
{
  ui->setupUi(this);

  LoadSettings();
  Calc();
  mTrayIcon->show();
//  mTrayIcon->showMessage(windowTitle(), "Работает система посчёта времени");

  mCalcTimer->start(kCalcPeriodMs);
  connect(mTrayIcon, &QSystemTrayIcon::activated, this, &MainWindow::OnTrayActivated);
  connect(mCalcTimer, &QTimer::timeout, this, &MainWindow::OnTimeout);
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::closeEvent(QCloseEvent* event)
{
  hide();
  event->ignore();
}

void MainWindow::Calc()
{
  mTodayUsed += mUseTimer.restart();
  mTodayLast  = mTodayLimit - mTodayUsed;

  ui->labelLimit->setText(FormatTimeRu(mTodayLimit));
  ui->labelUsed->setText(FormatTimeRu(mTodayUsed));
  ui->labelLast->setText(FormatTimeRu(mTodayLast));
  int percent = qMin(255, 255 * mTodayUsed / qMax(1, mTodayLimit));
  if (percent != mLastPercent) {
    mLastPercent = percent;
    if (mLastPercent < 255) {
      int   red = mLastPercent;
      int green = 255 - mLastPercent;
      ui->labelLast->setStyleSheet(QString("background-color: rgb(%2, %1, 0);").arg(green).arg(red));
      QImage greenIcon(":/Icons/Green.png");
      QImage newIcon(greenIcon.size(), QImage::Format_ARGB32);
      for (int j = 0; j < newIcon.height(); j++) {
        for (int i = 0; i < newIcon.width(); i++) {
          QColor oldColor = greenIcon.pixelColor(i, j);
          int   red = oldColor.green() * mTodayUsed / qMax(1, mTodayLimit);
          int green = oldColor.green() - red;

          newIcon.setPixelColor(i, j, QColor(red, green, oldColor.blue(), oldColor.alpha()));
        }
      }
      mTrayIcon->setIcon(QIcon(QPixmap::fromImage(newIcon)));
    } else {
      ui->labelLast->setStyleSheet("background-color: rgb(255, 0, 0);");
      mTrayIcon->setIcon(QIcon(":/Icons/Halt.png"));
    }
  }
  if (mTodayLimit > mTodayUsed) {
    mTrayIcon->setToolTip(QString("Осталось %1").arg(FormatTimeRu(mTodayLimit - mTodayUsed)));
  } else {
    mTrayIcon->setToolTip(QString("Доступного времени не осталось"));
  }

  mTodaySettings->setValue("Today", QDate::currentDate().toString("dd/MM/yyyy"));
  mTodaySettings->setValue("Used", mTodayUsed);
  mTodaySettings->sync();
}

void MainWindow::OnTrayActivated()
{
  show();
}

void MainWindow::OnTimeout()
{
  Calc();

  if (mTodayUsed > mTodayLimit) {
    mTrayIcon->showMessage(windowTitle()
                           , "Лимит времени исчерпан, необходимо выключить компьютер!"
                           , QSystemTrayIcon::Critical, kFatalMs);
  } else if (mTodayLimit - mTodayUsed < kWarningTimeMs) {
    mTrayIcon->showMessage(windowTitle()
                           , QString("Лимит времени за компьютером подходит к концу!\nОсталось %1").arg(FormatTimeRu(mTodayLimit - mTodayUsed))
                           , QSystemTrayIcon::Critical, kFatalMs);
  }
}

void MainWindow::on_buttonBoxMain_accepted()
{
  hide();
}

void MainWindow::LoadSettings()
{
  QDir appDir(QApplication::instance()->applicationDirPath());
  appDir.cd("Var");
  QString filename = appDir.absoluteFilePath(QString("%1.ini").arg(QApplication::instance()->applicationName()));
  QSettings settings(filename, QSettings::IniFormat);
  mCommonLimits.resize(7);
  for (int i = 0; i < mCommonLimits.size(); i++) {
    mCommonLimits[i] = settings.value(QString("DayLimit.%1").arg(i), i <= 5? kDefaultCommonLimit: kDefaultRedLimit).toInt();
  }
  for (int i = 0; ; i++) {
    QString dayText = settings.value(QString("Day.%1").arg(i)).toString();
    int        type = settings.value(QString("Type.%1").arg(i)).toInt();
    if (dayText.isEmpty()) {
      break;
    }
    QDate day = QDate::fromString(dayText, "dd/MM");
    mExceptionDayMap[day] = type;
  }

  QDate curDay = QDate::currentDate();
  if (mExceptionDayMap.contains(curDay)) {
    mDayType = qBound(0, mExceptionDayMap[curDay], 6);
  } else {
    mDayType = qBound(0, curDay.dayOfWeek() - 1, 6);
  }
  mTodayLimit = mCommonLimits.value(mDayType, kDefaultCommonLimit);

  mTodaySettings = new QSettings(appDir.absoluteFilePath(QString("Usage.ini")), QSettings::IniFormat, this);
  QString today = mTodaySettings->value("Today").toString();
  mTodayUsed    = mTodaySettings->value("Used").toInt();
  if (today != curDay.toString("dd/MM/yyyy")) {
    mTodayUsed = 0;
  }
  mUseTimer.start();
  mLastPercent = -1;
}
