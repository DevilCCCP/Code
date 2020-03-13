#include <QMessageBox>

#include <Lib/Include/QtAppGui.h>
#include <Lib/Common/Var.h>

#include "MainWindow.h"


const QString GetProgramName()
{ return QString("Db backup"); }

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  DbS db(new Db());
  if (int ret = ConnectPrimaryDb(*db, nullptr)) {
    return ret;
  }

  MainWindow w;
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

