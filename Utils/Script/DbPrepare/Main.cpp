#include <QCoreApplication>
#include <QDebug>

#include "DbPrepare.h"


int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (qApp->arguments().size() != 2) {
    qInfo() << "exe install_path";
    return 1;
  }

  QString installPath = qApp->arguments().value(1);
  DbPrepare dbPrepare(installPath);
  return dbPrepare.Exec();
}
