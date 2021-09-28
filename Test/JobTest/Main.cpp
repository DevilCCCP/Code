#include <QMessageBox>

#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Common/Var.h>
#include <Lib/Dispatcher/Overseer.h>

#include "MainWindow.h"
#include "MultiProcessCalc.h"


const QString GetProgramName()
{ return QString("Job Test"); }

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
    QString name = QString("Worker %1").arg(id);
    ObjectItemS obj = db->getObjectTable()->GetItem(QString("WHERE guid=%1").arg(ToSql(name))).dynamicCast<ObjectItem>();
    if (!obj) {
      obj.reset(new ObjectItem());
      obj->Name = name;
      obj->Guid = name;
      obj->Type = 1;
      db->getObjectTable()->InsertItem(obj);
    }
    if (!obj) {
      return 1;
    }
    Overseer overseer("test", obj->Id, true, true, true, QString(), QString());
    MultiProcessCalcS calc(new MultiProcessCalc(*db));
    overseer.RegisterWorker(calc);
    Log.SetFileLogging(QString("%1_").arg(id, 6, 10, QChar('0')));
    return overseer.Run();
  }

  MainWindow w(db);
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

