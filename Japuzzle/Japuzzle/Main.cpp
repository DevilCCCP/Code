#include <QApplication>

#include "MainWindow.h"
#include "Core.h"
#include "Decoration.h"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  MainWindow w;
  w.show();

  return app.exec();
}
