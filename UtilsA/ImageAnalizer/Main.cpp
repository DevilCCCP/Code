#include <QApplication>

#include "MainWindow.h"


const QString& GetProgramName()
{
  static QString gProgram(QStringLiteral("Image analizer"));
  return gProgram;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return a.exec();
}
