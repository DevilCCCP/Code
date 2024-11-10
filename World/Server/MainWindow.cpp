#include <QApplication>
#include <QScreen>
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

  ui->formMapPreview->hide();

  setWindowTitle(qCore->getProgramName());

  if (!Restore()) {
    QScreen* screen = qApp->primaryScreen();
    QRect geometry = screen->availableGeometry();
    move(geometry.topLeft());
    resize(geometry.size() - (frameSize() - size()));
    setWindowState(windowState() | Qt::WindowMaximized);
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::InitNewWorld()
{
  MapParameters mapParameters;
  mapParameters.setGlobeRadius(100);
  mapParameters.setGlobeSector(120);
  mapParameters.setWorldSector(120);
  mapParameters.setGroundPercent(30);
  mapParameters.setGlobePlateCount(3);
  mapParameters.setWorldPlateCount(3);
  ui->formMapPreview->Start(mapParameters);
}

void MainWindow::on_pushButtonNew_clicked()
{
  ui->widgetMenu->hide();
  ui->formMapPreview->show();

  InitNewWorld();
}
