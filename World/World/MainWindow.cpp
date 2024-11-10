#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QDateTime>
#include <QtMath>

#include <LibZ/FormEarth.h>
#include <LibZ/FormMapPreview.h>
#include <LibZ/MapGenerator.h>
#include <LibZ/MapParameters.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Core.h"


bool gTest = false;

MainWindow::MainWindow(QWidget* parent)
  : MainWindow2(parent), ui(new Ui::MainWindow)
  , mCore(new Core())
{
  Q_INIT_RESOURCE(Ui);

  ui->setupUi(this);

  setWindowTitle(qCore->getProgramName());

  if (!Restore()) {
    QScreen* screen = qApp->primaryScreen();
    QRect geometry = screen->availableGeometry();
    move(geometry.topLeft());
    resize(geometry.size() - (frameSize() - size()));
    setWindowState(windowState() | Qt::WindowMaximized);
  }

  ui->formEarth->hide();

//  ui->scrollAreaMain->hide();

//  ui->formEarth->SetShowAxes(true);

//  EarthLandscape earthLandscape;
//  EarthLevel terrainLevel;

//  if (gTest) {
//    EarthPlate plate;
//    plate.Color = QColor(Qt::darkGreen);
//    QVector<QPointF> border;
//    int radius = 20;
//    QPointF center(90, 0);
//    for (int j = 0; j < 360; j++) {
//      qreal alpha = j * M_PI / 180.0;
//      QPointF p = center + radius * QPointF(sin(alpha), cos(alpha));
//      border.append(p);
//    }
//    plate.Border = border;
//    terrainLevel.append(plate);

//    for (int j = -80; j <= 88; j += 2){
//      EarthPlate plateX;
//      plateX.Color = QColor(Qt::darkYellow);
//      QVector<QPointF> border;
//      for (int i = 5; i <= 180; i+=1) {
//        QPointF p(i, j);
//        border.prepend(p);
//      }
//      for (int i = 180; i >= 5; i-=1) {
//        QPointF p(i, j + 1);
//        border.prepend(p);
//      }
//      plateX.Border = border;
//      terrainLevel.append(plateX);
//    }

//    for (int j = 2; j < 360; j += 2){
//      EarthPlate plateY;
//      plateY.Color = QColor(Qt::darkCyan);
//      QVector<QPointF> border;
//      for (int i = 5; i <= 90; i++) {
//        QPointF p(j, i);
//        border.append(p);
//      }
//      for (int i = 90; i >= 5; i--) {
//        QPointF p(j + 1, i);
//        border.append(p);
//      }
//      plateY.Border = border;
//      terrainLevel.append(plateY);
//    }

//    earthLandscape.append(terrainLevel);
//    ui->formEarth->SetLandscape(earthLandscape);
//  }

  FormMapPreview* formMapPreview = new FormMapPreview(ui->scrollAreaWidgetContents);
  ui->scrollAreaWidgetContents->resize(800, 800);
  formMapPreview->resize(800, 800);
  formMapPreview->move(0, 0);

  MapParameters mapParameters;
  mapParameters.setGlobeSector(360);
  mapParameters.setGlobePlateCount(6);
  mapParameters.setWorldSector(120);
  mapParameters.setWorldPlateCount(2);
  formMapPreview->SetParameters(mapParameters);
}

MainWindow::~MainWindow()
{
  delete ui;
}
