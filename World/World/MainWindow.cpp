#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QtMath>

#include <LibZ/FormMapPreview.h>
#include <LibZ/MapParameters.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Core.h"


MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mCore(new Core())
{
  Q_INIT_RESOURCE(Ui);

  ui->setupUi(this);

  setWindowTitle(qCore->getProgramName());

  if (!Restore()) {
    QDesktopWidget* desktop = qApp->desktop();
    int primaryScreen = desktop->primaryScreen();
    QRect geometry = desktop->availableGeometry(primaryScreen);
    move(geometry.topLeft());
    resize(geometry.size() - (frameSize() - size()));
    setWindowState(windowState() | Qt::WindowMaximized);
  }

//  ui->scrollAreaMain->hide();

//  ui->formEarth->SetShowAxes(true);

//  EarthLandscape earthLandscape;
//  EarthLevel terrainLevel;

//  EarthPlate plate;
//  plate.Color = QColor(Qt::green);
//  QVector<QPointF> border;
//  int radius = 20;
//  QPointF center(90, 0);
//  for (int j = 10; j <= 350; j++) {
//    qreal alpha = j * M_PI / 180.0;
//    QPointF p = center + radius * QPointF(sin(alpha), cos(alpha));
//    border.append(p);
//  }
//  QPointF p1 = border.front();
//  QPointF p2 = border.back();
//  for (int j = 0; j < 60; j++) {
//    border.prepend(QPointF(p1.x(), p1.y() + j));
//    border.append(QPointF(p2.x(), p2.y() + j));
//  }
//  center.ry() += 60 + 2*radius;
//  for (int j = 10; j <= 350; j++) {
//    qreal alpha = (180 + j) * M_PI / 180.0;
//    QPointF p = center + radius * QPointF(sin(alpha), cos(alpha));
//    border.append(p);
//  }
////  QVector<QPointF> borderr;
////  for (const QPointF& p: border) {
////    borderr.prepend(p);
////  }
//  plate.Border = border;
//  terrainLevel.append(plate);

////  qsrand(QDateTime::currentMSecsSinceEpoch());
////  for (int i = 0; i < 6; i++) {
////    EarthPlate plate;
////    plate.Color = QColor(Qt::green);
////    int radius = 2 + qrand() % 20;
////    QPointF center(radius + qrand() % (180 - 2*radius), radius + qrand() % (360 - 2*radius));
////    for (int j = 0; j < 360; j++) {
////      qreal alpha = j * M_PI / 180.0;
////      QPointF p = center + radius * QPointF(sin(alpha), cos(alpha));
////      plate.Border.append(p);
////    }
////    terrainLevel.append(plate);
////  }
//  earthLandscape.append(terrainLevel);
//  ui->formEarth->SetLandscape(earthLandscape);
////  FormMapPreview* formMapPreview = new FormMapPreview(ui->scrollAreaWidgetContents);
////  formMapPreview->resize(800, 800);
////  formMapPreview->move(0, 0);
////  ui->scrollAreaWidgetContents->resize(800, 800);

////  MapParameters mapParameters;
////  mapParameters.setGlobeSector(360);
////  mapParameters.setGlobePlateCount(6);
////  mapParameters.setWorldSector(120);
////  mapParameters.setWorldPlateCount(2);
////  formMapPreview->SetParameters(mapParameters);
}

MainWindow::~MainWindow()
{
  delete ui;
}
