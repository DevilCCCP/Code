#include <QMessageBox>

#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Common/Var.h>
#include <Lib/Dispatcher/Overseer.h>

#include "MainWindow.h"
#include "MultiProcessCalc.h"


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

  if (qApp->arguments().size() == 3 && qApp->arguments().at(1) == "worker") {
    int id = qApp->arguments().at(2).toInt();
    ObjectItemS obj(new ObjectItem());
    obj->Id = id;
    obj->Name = QString("Worker %1").arg(id);
    obj->Guid = obj->Name;
    obj->Type = 1;
    db->getObjectTable()->InsertItem(obj);
    Overseer overseer("test", id, true, true, true, QString(), QString());
    MultiProcessCalcS calc(new MultiProcessCalc(*db));
    overseer.RegisterWorker(calc);
    db->MoveToThread(calc.data());
    return overseer.Run();
  }

  MainWindow w(db);
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

