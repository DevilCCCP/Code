#include <qsystemdetection.h>

#ifdef Q_OS_WIN32
#define ANAL_DEBUG
#endif

#if defined(ANAL_DEBUG) || defined(USE_VDPAU)
#include <Lib/Include/QtAppWin.h>
#else
#include <Lib/Include/QtAppCon.h>
#endif
#include <Lib/Common/Uri.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Net/Listener.h>
#include <LibV/Decoder/Decoder.h>
#include <LibV/Storage/Storage.h>
#include <LibV/Source/Source.h>
#include <LibV/Va/Analizer.h>
#include <LibV/Source/Ptz.h>
#include <Local/ModuleNames.h>

#include "Saver.h"
#include "Transmit.h"


int qmain(int argc, char* argv[])
{
  DbS db;
  OverseerS overseer;
  if (int ret = Overseer::ParseMainWithDb(kServerDaemon, argc, argv, overseer, db)) {
    return ret;
  }

  int repoId = 0;
  int ptzId = 0;
  int analId = 0;
  QString vaType;
  auto q = db->MakeQuery();
  q->prepare(QString("SELECT t.name, o._id, o.status FROM object_connection c"
                     " INNER JOIN object o ON o._id = c._oslave"
                     " INNER JOIN object_type t ON t._id = o._otype"
                     " WHERE c._omaster = %1").arg(overseer->Id()));
  if (!db->ExecuteQuery(q)) {
    return -2101;
  }
  while (q->next()) {
    QString typeName = q->value(0).toString();
    int id = q->value(1).toInt();
    int state = q->value(2).toInt();
    if (state == 0) {
      if (typeName == "rep") {
        if (repoId) {
          Log.Warning(QString("Too many repos connected (last: %1, this: %2)").arg(repoId).arg(id));
        }
        repoId = id;
      } else if (typeName.startsWith("va")) {
        if (analId) {
          Log.Warning(QString("Too many va connected (last: %1, this: %2)").arg(analId).arg(id));
        } else {
          vaType = typeName;
          analId = id;
        }
      } else if (typeName == "ptz") {
        if (ptzId) {
          Log.Warning(QString("Too many PTZ connected (last: %1, this: %2)").arg(ptzId).arg(id));
        }
        ptzId = id;
      }
    }
  }

  DbSettings cameraSettings(*db);
  if (!cameraSettings.Open(QString::number(overseer->Id()))) {
    return -2100;
  }

  SourceS source = (overseer->Params() != "child")? Source::CreateSource(cameraSettings, overseer->Quiet()): Source::CreateChildSource();
  overseer->RegisterWorker(source);

  PtzS ptz;
  if (ptzId) {
    DbSettings ptzSettings(*db);
    if (!ptzSettings.Open(QString::number(ptzId))) {
      return -2300;
    }

    ptz = Ptz::CreatePtz(ptzSettings);
    overseer->RegisterWorker(ptz);
  }

  Uri uri = Uri::FromString(overseer->Uri());
  if (uri.Type() != Uri::eTcp) {
    Log.Fatal(QString("Can't create player without valid URI (uri: %1)").arg(overseer->Uri()));
    overseer->Restart();
    return -2101;
  }

  AnalizerS analizer;
  DecoderS decoder;
  CtrlWorker* outPin;
  if (analId) {
    bool withDecode = source->NeedDecoder();
    if (withDecode) {
      decoder = DecoderS(new Decoder(true, false));
      int fps = cameraSettings.GetValue("Fps", 0).toInt();
      int decodeType = cameraSettings.GetValue("Decode", 0).toInt();
      decoder->setFps(fps);
      decoder->setUseHardware(decodeType == 1);
      overseer->RegisterWorker(decoder);
      source->ConnectModule(decoder.data());
    }

    analizer.reset(new Analizer(db, analId, vaType, withDecode, overseer->Params() == "test_va" && overseer->Detached()));
    if (int ret = analizer->LoadSettings(db, analId)) {
      return -2200 + ret;
    }
    overseer->RegisterWorker(analizer);

    source->ConnectModule(analizer.data());
    if (decoder) {
      decoder->ConnectModule(analizer.data());
    }

    outPin = analizer.data();
  } else {
    outPin = source.data();
  }

  // setup storage
  SaverS saver;
  SettingsAS storageSettings;
  if (repoId) {
    DbSettingsS settings = DbSettingsS(new DbSettings(*db));
    if (settings->Open(QString::number(repoId))) {
      storageSettings = Storage::CopySettings(Storage::ConnectionFile(repoId), *settings);

      saver = SaverS(new Saver(storageSettings));
      overseer->RegisterWorker(saver);
      outPin->ConnectModule(saver.data());
    } else {
      Log.Error("Storage settings open fail");
      storageSettings.clear();
    }
  }

  int poolLimit = cameraSettings.GetValue("PoolLimit", 12).toInt();
  TransmitS transmit(new Transmit(storageSettings, uri.Port(), poolLimit));
  transmit->setPtz(ptz);
  overseer->RegisterWorker(transmit);
  outPin->ConnectModule(transmit.data());
  if (analId) {
    if (decoder) {
      transmit->SetThumbnail(decoder->GetThumbnail());
    } else {
      transmit->SetThumbnail(source->GetThumbnail());
    }
  }

  if (analizer) {
    db->MoveToThread(analizer.data());
  }
  return overseer->Run();
}

