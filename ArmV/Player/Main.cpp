#include <Lib/Include/QtAppWin.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Decoder/Decoder.h>
#include <LibV/Include/ModuleNames.h>
#include <LibV/Player/Render.h>
#include <LibV/Player/Drawer.h>

#include "DownloadDialog.h"


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int DownloadPlayer(const QString& params, const DbS& db, const OverseerS& overseer);

int qmain(int argc, char* argv[])
{
  DbS db;
  OverseerS overseer;
  if (int ret = Overseer::ParseMainWithDb(kArmDaemon, argc, argv, overseer, db)) {
    return ret;
  }
  const QString& oparams = overseer->Params();

  QString commandDownload("download;");
  QString commandCustom("custom;");
  QString commandSingle("single;");
  if (oparams.startsWith(commandDownload)) {
    return DownloadPlayer(oparams.mid(commandDownload.size()), db, overseer);
  }

  DrawerS drawer(new Drawer);
  RenderS render(new Render(*db, drawer));
  if (oparams.startsWith(commandCustom)) {
    QString params = oparams.mid(commandCustom.size());
    QStringList plist = params.split(";", QString::SkipEmptyParts);
    bool ok1, ok2, ok3, ok4, ok5, ok6;
    ok1 = ok2 = ok3 = ok4 = ok5 = ok6 = false;
    int id, type, count, monitor, camId, armId;
    qint64 ts = 0;
    for (auto itr = plist.begin(); itr != plist.end(); itr++) {
      const QString& kvText = *itr;
      QStringList kv = kvText.split('=', QString::KeepEmptyParts);
      if (kv.size() == 2) {
        const QString& key = kv.at(0);
        const QString& value = kv.at(1);
        //custom;id=%1;type=%2;cnt=%3;mon=%4;cam=%5;obj=%6
        if (key == "id") {
          id = value.toInt(&ok1);
        } else if (key == "type") {
          type = value.toInt(&ok2);
        } else if (key == "cnt") {
          count = value.toInt(&ok3);
        } else if (key == "mon") {
          monitor = value.toInt(&ok4);
        } else if (key == "cam") {
          camId = value.toInt(&ok5);
        } else if (key == "obj") {
          armId = value.toInt(&ok6);
        } else if (key == "ts") {
          ts = value.toLongLong();
        }
      }
    }
    if (!ok1 || !ok2 || !ok3 || !ok4 || !ok5 || !ok6) {
      Log.Fatal("Invalid parameters");
      return -3012;
    }
    if (!render->InitCustomScene(id, type, count, monitor, camId, armId, ts)) {
      Log.Fatal("Invalid init parameters");
      return -3013;
    }
  } else if (oparams.startsWith(commandSingle)) {
    QString params = oparams.mid(commandCustom.size());
    QStringList plist = params.split(";", QString::SkipEmptyParts);
    bool ok1, ok2;
    ok1 = ok2 = false;
    int camId, armId;
    for (auto itr = plist.begin(); itr != plist.end(); itr++) {
      const QString& kvText = *itr;
      QStringList kv = kvText.split('=', QString::KeepEmptyParts);
      if (kv.size() == 2) {
        const QString& key = kv.at(0);
        const QString& value = kv.at(1);
        //single;cam=%1;arm=%2
        if (key == "cam") {
          camId = value.toInt(&ok1);
        } else if (key == "arm") {
          armId = value.toInt(&ok2);
        }
      }
    }
    if (!ok1 || !ok2) {
      Log.Fatal("Invalid parameters");
      return -3022;
    }
    if (!render->InitSingleScene(camId, armId)) {
      Log.Fatal("Invalid init parameters");
      return -3023;
    }
  }
  overseer->RegisterWorker(render);

  DecoderS decoder(new Decoder(false, true));
  overseer->RegisterWorker(decoder);
  render->ConnectModule(decoder.data());

  decoder->ConnectModule(drawer.data());
  overseer->RegisterWorker(drawer);
  return overseer->Run();
}

